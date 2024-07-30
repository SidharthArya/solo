#ifndef SOLO_EVENTS_H
#define SOLO_EVENTS_H
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

void xdg_toplevel_map(struct wl_listener *listener, void *data);

void xdg_toplevel_unmap(struct wl_listener *listener, void *data);

void xdg_toplevel_commit(struct wl_listener *listener, void *data);

void xdg_toplevel_destroy(struct wl_listener *listener, void *data);
void begin_interactive(struct solo_toplevel *toplevel,
                       enum solo_cursor_mode mode, uint32_t edges);
void xdg_toplevel_request_move(struct wl_listener *listener, void *data);

void xdg_toplevel_request_resize(struct wl_listener *listener, void *data);

void xdg_toplevel_request_maximize(struct wl_listener *listener, void *data);

void xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data);

void server_new_xdg_toplevel(struct wl_listener *listener, void *data);

void xdg_popup_commit(struct wl_listener *listener, void *data);

void xdg_popup_destroy(struct wl_listener *listener, void *data);

void server_new_xdg_popup(struct wl_listener *listener, void *data);
#endif
