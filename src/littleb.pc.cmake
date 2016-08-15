prefix=@CMAKE_INSTALL_PREFIX@
exec_prefix=${prefix}
libdir=${exec_prefix}/@LIB_INSTALL_DIR@
includedir=${prefix}/include

Name: littleb
Description: Low Level Skeleton Library for Communication
Version: @littleb_VERSION_STRING@

Libs: -L${libdir} -llittleb
Cflags: -I${includedir}
