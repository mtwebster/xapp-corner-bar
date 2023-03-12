glib_min_ver = '>=2.44.0'

common_deps = []
common_deps += dependency('gio-2.0', version: glib_min_ver, required: true)
common_deps += dependency('glib-2.0', version: glib_min_ver, required: true)
common_deps += dependency('gtk+-3.0', version: '>=3.16', required: true)
common_deps += dependency('libwnck-3.0', required: true)

common_gresources = gnome.compile_resources(
  'xapp-corner-bar-resources', 'xapp-corner-bar.gresource.xml',
  source_dir: '../theme',
  c_name: 'xapp_corner_bar',
  install_header: false,
  install: false,
  dependencies: theme_sources
)

mate_deps = common_deps
mate_deps += dependency('libmatepanelapplet-4.0', required: true)

mate_corner_bar_sources = [
    'common.c',
    'mate-xapp-corner-bar.c'
]
mate_corner_bar_applet = executable(
    'mate-xapp-corner-bar',
    sources: [ mate_corner_bar_sources, common_gresources ],
    dependencies: mate_deps,
    include_directories: top_inc,
    install_dir: mate_applet_install_dir,
    install: true,
)

xfce4_deps = common_deps
xfce4_deps += dependency('libxfce4panel-2.0', version: '>=4.12.2', required: true)

xfce4_corner_bar_sources = [
    'common.c',
    'xfce4-xapp-corner-bar-plugin.c'
]

xfce4_corner_bar_plugin = shared_module('xfce4-xapp-corner-bar-plugin',
    sources  : [ xfce4_corner_bar_sources, common_gresources ],
    include_directories: [top_inc],
    dependencies: xfce4_deps,
    c_args: [
        '-DG_LOG_DOMAIN="XAppCornerBar"'
    ],
    link_args: [ '-Wl,-Bsymbolic', '-Wl,-z,relro', '-Wl,-z,now', '-lm'
    ],
    install: true,
    install_dir: join_paths(get_option('libdir'), 'xfce4', 'panel', 'plugins')
)

