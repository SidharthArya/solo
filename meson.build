project('solo',  'c')
add_global_arguments('-DWLR_USE_UNSTABLE=true', language : 'c')

incdir = include_directories(['src', 'src/lib'])
executable(
	'solo',
	['src/solo/main.c', ],include_directories : incdir,
  dependencies : [dependency('wlroots-0.18'), dependency('wayland-server'), dependency('xkbcommon')]
)

