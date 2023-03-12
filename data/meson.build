### MATE
conf = configuration_data()
conf.set('install_dir', mate_applet_install_dir)

service_file = configure_file(
  input : 'org.mate.panel.applet.MateXAppCornerBarAppletFactory.service.in',
  output: 'org.mate.panel.applet.MateXAppCornerBarAppletFactory.service',
  configuration: conf,
)

install_data(service_file,
  install_dir: join_paths(get_option('datadir'), 'dbus-1', 'services')
)

def_file = configure_file(
  input: 'org.x.MateXAppCornerBarApplet.mate-panel-applet.in.in',
  output: 'org.x.MateXAppCornerBarApplet.mate-panel-applet.in',
  configuration: conf,
)

i18n.merge_file(
  input: def_file,
  output: 'org.x.MateXAppCornerBarApplet.mate-panel-applet',
  type: 'desktop',
  po_dir: join_paths(meson.source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('datadir'), 'mate-panel', 'applets')
)

### XFCE4

i18n.merge_file(
    input: 'xfce4-xapp-corner-bar-plugin.desktop.in',
    output: 'xfce4-xapp-corner-bar-plugin.desktop',
    type: 'desktop',
    po_dir: join_paths(meson.source_root(), 'po'),
    install: true,
    install_dir: join_paths(get_option('datadir'), 'xfce4', 'panel', 'plugins')
)

### GSettings
install_data('org.x.apps.corner-bar.gschema.xml',
    install_dir: join_paths(get_option('datadir'), 'glib-2.0', 'schemas')
)
meson.add_install_script('meson_install_schemas.py')

subdir('icons')