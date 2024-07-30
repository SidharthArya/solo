project('solo',  'c')
add_global_arguments('-DWLR_USE_UNSTABLE=true', language : 'c')
deps = [dependency('wlroots-0.18'), dependency('wayland-server'), dependency('xkbcommon')]
incdir = include_directories(['src'])
srcdir = 'src/'
slib = shared_library('solo', sources: [
    srcdir + 'inputs/keyboard.h',
    srcdir + 'inputs/keyboard.c',
    srcdir + 'inputs/cursor.h',
    srcdir + 'inputs/cursor.c',
    srcdir + 'node/node.h',
    srcdir + 'node/node.c',
    srcdir + 'display/display.h',
    srcdir + 'display/display.c',
    srcdir + 'events/events.h',
    srcdir + 'events/events.c',
    ], include_directories: incdir,
    dependencies: deps
    )

executable(
	'solo',
	['src/solo/main.c'],include_directories : incdir,
  dependencies : deps,
  link_with: slib)

