#ifndef SOLO_CURSOR_H
#define SOLO_CURSOR_H
#include <assert.h>
#include <lib/solo.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
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

void server_new_pointer(struct solo_server *server,
                        struct wlr_input_device *device);

void server_new_input(struct wl_listener *listener, void *data);

void seat_request_cursor(struct wl_listener *listener, void *data);

void seat_request_set_selection(struct wl_listener *listener, void *data);

struct solo_toplevel *desktop_toplevel_at(struct solo_server *server, double lx,
                                          double ly,
                                          struct wlr_surface **surface,
                                          double *sx, double *sy);

void reset_cursor_mode(struct solo_server *server);

void process_cursor_move(struct solo_server *server, uint32_t time);

void process_cursor_resize(struct solo_server *server, uint32_t time);

void process_cursor_motion(struct solo_server *server, uint32_t time);

void server_cursor_motion(struct wl_listener *listener, void *data);

void server_cursor_motion_absolute(struct wl_listener *listener, void *data);

void server_cursor_button(struct wl_listener *listener, void *data);

void server_cursor_axis(struct wl_listener *listener, void *data);

void server_cursor_frame(struct wl_listener *listener, void *data);
#endif
