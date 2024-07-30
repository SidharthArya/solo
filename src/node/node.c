#include <getopt.h>
#include <inputs/cursor.h>
#include <inputs/keyboard.h>
#include <lib/solo.h>
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

struct solo_toplevel *desktop_toplevel_at(struct solo_server *server, double lx,
                                          double ly,
                                          struct wlr_surface **surface,
                                          double *sx, double *sy) {
  /* This returns the topmost node in the scene at the given layout coords.
   * We only care about surface nodes as we are specifically looking for a
   * surface in the surface tree of a solo_toplevel. */
  struct wlr_scene_node *node =
      wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }
  struct wlr_scene_buffer *scene_buffer = wlr_scene_rect_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_try_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;
  /* Find the node corresponding to the solo_toplevel at the root of this
   * surface tree, it is the only one for which we set the data field. */
  struct wlr_scene_tree *tree = node->parent;
  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }
  return tree->node.data;
}
