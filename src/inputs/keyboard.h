#ifndef SOLO_KEYBOARD_H
#define SOLO_KEYBOARD_H

#include <lib/solo.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>

void focus_toplevel(struct solo_toplevel *toplevel,
                    struct wlr_surface *surface);

void keyboard_handle_modifiers(struct wl_listener *listener, void *data);

bool handle_keybinding(struct solo_server *server, xkb_keysym_t sym);

void keyboard_handle_key(struct wl_listener *listener, void *data);

void keyboard_handle_destroy(struct wl_listener *listener, void *data);

void server_new_keyboard(struct solo_server *server,
                         struct wlr_input_device *device);

#endif
