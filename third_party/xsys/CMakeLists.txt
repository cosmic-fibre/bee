
project ( xsys )
include ( cmake/dist.cmake )
include ( lua )

install ( DIRECTORY xsys DESTINATION ${MODULE_LUADIR} )
