project('xapp-corner-bar',
    'c',
    version : '1.0.0',
    default_options : [ 'buildtype=debugoptimized' ],
)

gnome = import('gnome')
pkg = import('pkgconfig')
i18n = import('i18n')

locale_dir = join_paths(get_option('prefix'), get_option('localedir'))

cdata = configuration_data()
cdata.set_quoted('GETTEXT_PACKAGE', 'xapp-corner-bar')
cdata.set_quoted('LOCALEDIR', locale_dir)
cdata.set_quoted('PREFS_PATH', join_paths(get_option('prefix'), get_option('bindir'), 'xapp-corner-bar-prefs'))
cdata.set_quoted('VERSION', meson.project_version())
cdata.set10('HAVE_X11', true)
cdata.set('ENABLE_DEBUG', get_option('buildtype') in ['debug', 'debugoptimized'])

mate_applet_install_dir = join_paths(get_option('prefix'), get_option('libdir'), 'xapps')

add_global_arguments([
    '-Wunused',
    '-Wimplicit-function-declaration'
  ],
  language: 'c'
)

if not get_option('deprecated_warnings')
  add_global_arguments([
      '-Wno-deprecated-declarations',
      '-Wno-deprecated',
      '-Wno-declaration-after-statement',
    ],
    language: 'c',
  )
endif

c = configure_file(output : 'config.h',
    configuration : cdata
)

top_inc = include_directories('.')

theme_sources = []
subdir('theme')
subdir('src')
subdir('data')
subdir('po')
subdir('bin')
