
project ( lpeg C )
include ( cmake/dist.cmake )
include ( lua )

install_lua_module ( ${MODULE_LIBDIR}/lpeg lpvm.c lpcap.c lptree.c lpcode.c lpprint.c lpeg.def )
install_lua_module ( ${MODULE_LIBDIR}/re re.lua )

