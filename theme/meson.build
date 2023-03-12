fs = import('fs')
sassc = find_program('sassc')

theme_sources += custom_target(
  'xapp-corner-bar.css',
  input: '_style.scss',
  output: 'xapp-corner-bar.css',
  command: [
    sassc, '-a', '@INPUT@', '@OUTPUT@'
  ],
  build_always: true
)
