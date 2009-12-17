require 'mkmf'

dir_config("icu")

$LIBS << ' ' + `icu-config --ldflags-system --ldflags-libsonly`.strip
$LDFLAGS <<  ' ' + `icu-config --ldflags-searchpath`.strip

create_makefile("uchardet")
