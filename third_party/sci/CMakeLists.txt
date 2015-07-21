


install ( DIRECTORY sci/alg DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/dist DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/fmin DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/mcmc DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/qrng DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/quad DESTINATION ${MODULE_LUADIR}/sci )
install ( DIRECTORY sci/root DESTINATION ${MODULE_LUADIR}/sci )


install ( FILES sci/alg.lua    DESTINATION                ${MODULE_LUADIR}/sci                   )
install ( FILES sci/math.lua    DESTINATION                ${MODULE_LUADIR}/sci                   )
install ( FILES sci/dist.lua   DESTINATION             ${MODULE_LUADIR}/sci                   )
install ( FILES sci/root.lua   DESTINATION             ${MODULE_LUADIR}/sci                   )
install ( FILES sci/diff.lua   DESTINATION             ${MODULE_LUADIR}/sci           )
install ( FILES sci/fmax.lua   DESTINATION             ${MODULE_LUADIR}/sci           )
install ( FILES sci/fmin.lua   DESTINATION             ${MODULE_LUADIR}/sci             )
install ( FILES sci/mcmc.lua   DESTINATION             ${MODULE_LUADIR}/sci         )
install ( FILES sci/prng.lua   DESTINATION             ${MODULE_LUADIR}/sci        )
install ( FILES sci/stat.lua   DESTINATION             ${MODULE_LUADIR}/sci             )
install ( FILES sci/qrng.lua   DESTINATION            ${MODULE_LUADIR}/sci     )
install ( FILES sci/quad.lua   DESTINATION             ${MODULE_LUADIR}/sci  )
