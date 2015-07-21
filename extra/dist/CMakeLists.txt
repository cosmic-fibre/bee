# Scripts for a beectl packaging
set(SYSV_INITD_PATH ${CMAKE_INSTALL_LIBDIR}/bee/bee.init)
configure_file("bee.service.in" "bee.service")

if (ENABLE_RPM)
    # chmod +x 655
    if (WITH_SYSTEMD)
    message (STATUS "Building RPM with SYSTEMD")
        install (FILES bee.service DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR}
            PERMISSIONS
            OWNER_READ OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
        install (FILES bee.init DESTINATION ${CMAKE_INSTALL_LIBDIR}/bee/
            PERMISSIONS
            OWNER_READ OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    else (WITH_SYSTEMD)
    message (STATUS "Building RPM without SYSTEMD")
        install (FILES bee.init DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/init.d/
            RENAME bee
            PERMISSIONS
            OWNER_READ OWNER_WRITE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE)
    endif(WITH_SYSTEMD)
    install (FILES default/bee DESTINATION
        ${CMAKE_INSTALL_SYSCONFDIR}/sysconfig/
    PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ)
else (ENABLE_RPM)
    install (FILES default/bee DESTINATION
        ${CMAKE_INSTALL_SYSCONFDIR}/default/
    PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ)
endif(ENABLE_RPM)

install (FILES beectl DESTINATION ${CMAKE_INSTALL_BINDIR}
    PERMISSIONS
    OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)

pod2man (
    ${CMAKE_SOURCE_DIR}/extra/dist/beectl
    "beectl" 1 extra/dist
    "beectl instances control"
)

