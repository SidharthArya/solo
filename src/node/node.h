#ifndef SOLO_NODE_H
#define SOLO_NODE_H
#include <lib/solo.h>

struct solo_toplevel *desktop_toplevel_at(struct solo_server *server, double lx,
                                          double ly,
                                          struct wlr_surface **surface,
                                          double *sx, double *sy);

#endif
