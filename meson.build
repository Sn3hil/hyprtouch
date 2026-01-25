project('hyprtouch', 'cpp',
  version : '0.1.0',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

gtk4_dep = dependency('gtk4')
gtk4_layer_shell_dep = dependency('gtk4-layer-shell-0')
json_dep = dependency('nlohmann_json')

sources = files(
  'src/main.cpp',
  'src/input/handler.cpp',
  'src/mouse/coords.cpp',
  'src/mouse/control.cpp',
  'src/hypr/monitor.cpp',
  'src/overlay/window.cpp',
  'src/overlay/grid.cpp',
  'src/ipc/socket.cpp',
  'src/ipc/server.cpp',
  'src/ipc/client.cpp',
  'src/ipc/protocol.cpp',
  'src/daemon/daemon.cpp',
  'src/daemon/lockfile.cpp',
)

executable('hyprtouch',
  sources,
  dependencies : [gtk4_dep, gtk4_layer_shell_dep, json_dep],
  install : true)
