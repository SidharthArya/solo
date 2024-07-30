#include <assert.h>
#include <display/display.h>
#include <getopt.h>
#include <inputs/cursor.h>
#include <inputs/keyboard.h>
#include <lib/solo.h>
#include <node/node.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

void xdg_toplevel_map(struct wl_listener *listener, void *data) {
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct solo_toplevel *toplevel = wl_container_of(listener, toplevel, map);

  wl_list_insert(&toplevel->server->toplevels, &toplevel->link);

  focus_toplevel(toplevel, toplevel->xdg_toplevel->base->surface);
}

void xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
  /* Called when the surface is unmapped, and should no longer be shown. */
  struct solo_toplevel *toplevel = wl_container_of(listener, toplevel, unmap);

  /* Reset the cursor mode if the grabbed toplevel was unmapped. */
  if (toplevel == toplevel->server->grabbed_toplevel) {
    reset_cursor_mode(toplevel->server);
  }

  wl_list_remove(&toplevel->link);
}

void xdg_toplevel_commit(struct wl_listener *listener, void *data) {
  /* Called when a new surface state is committed. */
  struct solo_toplevel *toplevel = wl_container_of(listener, toplevel, commit);

  if (toplevel->xdg_toplevel->base->initial_commit) {
    /* When an xdg_surface performs an initial commit, the compositor must
     * reply with a configure so the client can map the surface. solo
     * configures the xdg_toplevel with 0,0 size to let the client pick the
     * dimensions itself. */
    wlr_xdg_toplevel_set_size(toplevel->xdg_toplevel, 0, 0);
  }
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
  /* Called when the xdg_toplevel is destroyed. */
  struct solo_toplevel *toplevel = wl_container_of(listener, toplevel, destroy);

  wl_list_remove(&toplevel->map.link);
  wl_list_remove(&toplevel->unmap.link);
  wl_list_remove(&toplevel->commit.link);
  wl_list_remove(&toplevel->destroy.link);
  wl_list_remove(&toplevel->request_move.link);
  wl_list_remove(&toplevel->request_resize.link);
  wl_list_remove(&toplevel->request_maximize.link);
  wl_list_remove(&toplevel->request_fullscreen.link);

  free(toplevel);
}
void begin_interactive(struct solo_toplevel *toplevel,
                       enum solo_cursor_mode mode, uint32_t edges) {
  /* This function sets up an interactive move or resize operation, where the
   * compositor stops propegating pointer events to clients and instead
   * consumes them itself, to move or resize windows. */
  struct solo_server *server = toplevel->server;
  struct wlr_surface *focused_surface =
      server->seat->pointer_state.focused_surface;
  if (toplevel->xdg_toplevel->base->surface !=
      wlr_surface_get_root_surface(focused_surface)) {
    /* Deny move/resize requests from unfocused clients. */
    return;
  }
  server->grabbed_toplevel = toplevel;
  server->cursor_mode = mode;

  if (mode == solo_CURSOR_MOVE) {
    server->grab_x = server->cursor->x - toplevel->scene_tree->node.x;
    server->grab_y = server->cursor->y - toplevel->scene_tree->node.y;
  } else {
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(toplevel->xdg_toplevel->base, &geo_box);

    double border_x = (toplevel->scene_tree->node.x + geo_box.x) +
                      ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (toplevel->scene_tree->node.y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    server->grab_x = server->cursor->x - border_x;
    server->grab_y = server->cursor->y - border_y;

    server->grab_geobox = geo_box;
    server->grab_geobox.x += toplevel->scene_tree->node.x;
    server->grab_geobox.y += toplevel->scene_tree->node.y;

    server->resize_edges = edges;
  }
}
void xdg_toplevel_request_move(struct wl_listener *listener, void *data) {
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct solo_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_move);
  begin_interactive(toplevel, solo_CURSOR_MOVE, 0);
}

void xdg_toplevel_request_resize(struct wl_listener *listener, void *data) {
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct solo_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_resize);
  begin_interactive(toplevel, solo_CURSOR_RESIZE, event->edges);
}

void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data) {
  /* This event is raised when a client would like to maximize itself,
   * typically because the user clicked on the maximize button on client-side
   * decorations. solo doesn't support maximization, but to conform to
   * xdg-shell protocol we still must send a configure.
   * wlr_xdg_surface_schedule_configure() is used to send an empty reply.
   * However, if the request was sent before an initial commit, we don't do
   * anything and let the client finish the initial surface setup. */
  struct solo_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_maximize);
  if (toplevel->xdg_toplevel->base->initialized) {
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  }
}

void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data) {
  /* Just as with request_maximize, we must send a configure here. */
  struct solo_toplevel *toplevel =
      wl_container_of(listener, toplevel, request_fullscreen);
  if (toplevel->xdg_toplevel->base->initialized) {
    wlr_xdg_surface_schedule_configure(toplevel->xdg_toplevel->base);
  }
}

void server_new_xdg_toplevel(struct wl_listener *listener, void *data) {
  /* This event is raised when a client creates a new toplevel (application
   * window). */
  struct solo_server *server =
      wl_container_of(listener, server, new_xdg_toplevel);
  struct wlr_xdg_toplevel *xdg_toplevel = data;

  /* Allocate a solo_toplevel for this surface */
  struct solo_toplevel *toplevel = calloc(1, sizeof(*toplevel));
  toplevel->server = server;
  toplevel->xdg_toplevel = xdg_toplevel;
  toplevel->scene_tree = wlr_scene_xdg_surface_create(
      &toplevel->server->scene->tree, xdg_toplevel->base);
  toplevel->scene_tree->node.data = toplevel;
  xdg_toplevel->base->data = toplevel->scene_tree;

  /* Listen to the various events it can emit */
  toplevel->map.notify = xdg_toplevel_map;
  wl_signal_add(&xdg_toplevel->base->surface->events.map, &toplevel->map);
  toplevel->unmap.notify = xdg_toplevel_unmap;
  wl_signal_add(&xdg_toplevel->base->surface->events.unmap, &toplevel->unmap);
  toplevel->commit.notify = xdg_toplevel_commit;
  wl_signal_add(&xdg_toplevel->base->surface->events.commit, &toplevel->commit);

  toplevel->destroy.notify = xdg_toplevel_destroy;
  wl_signal_add(&xdg_toplevel->events.destroy, &toplevel->destroy);

  /* cotd */
  toplevel->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&xdg_toplevel->events.request_move, &toplevel->request_move);
  toplevel->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&xdg_toplevel->events.request_resize,
                &toplevel->request_resize);
  toplevel->request_maximize.notify = xdg_toplevel_request_maximize;
  wl_signal_add(&xdg_toplevel->events.request_maximize,
                &toplevel->request_maximize);
  toplevel->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
  wl_signal_add(&xdg_toplevel->events.request_fullscreen,
                &toplevel->request_fullscreen);
}

void xdg_popup_commit(struct wl_listener *listener, void *data) {
  /* Called when a new surface state is committed. */
  struct solo_popup *popup = wl_container_of(listener, popup, commit);

  if (popup->xdg_popup->base->initial_commit) {
    /* When an xdg_surface performs an initial commit, the compositor must
     * reply with a configure so the client can map the surface.
     * solo sends an empty configure. A more sophisticated compositor
     * might change an xdg_popup's geometry to ensure it's not positioned
     * off-screen, for example. */
    wlr_xdg_surface_schedule_configure(popup->xdg_popup->base);
  }
}

void xdg_popup_destroy(struct wl_listener *listener, void *data) {
  /* Called when the xdg_popup is destroyed. */
  struct solo_popup *popup = wl_container_of(listener, popup, destroy);

  wl_list_remove(&popup->commit.link);
  wl_list_remove(&popup->destroy.link);

  free(popup);
}

void server_new_xdg_popup(struct wl_listener *listener, void *data) {
  /* This event is raised when a client creates a new popup. */
  struct wlr_xdg_popup *xdg_popup = data;

  struct solo_popup *popup = calloc(1, sizeof(*popup));
  popup->xdg_popup = xdg_popup;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  struct wlr_xdg_surface *parent =
      wlr_xdg_surface_try_from_wlr_surface(xdg_popup->parent);
  assert(parent != NULL);
  struct wlr_scene_tree *parent_tree = parent->data;
  xdg_popup->base->data =
      wlr_scene_xdg_surface_create(parent_tree, xdg_popup->base);

  popup->commit.notify = xdg_popup_commit;
  wl_signal_add(&xdg_popup->base->surface->events.commit, &popup->commit);

  popup->destroy.notify = xdg_popup_destroy;
  wl_signal_add(&xdg_popup->events.destroy, &popup->destroy);
}
