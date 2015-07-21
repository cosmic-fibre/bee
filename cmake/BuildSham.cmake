# A macro to build the bundled sham library
#
macro(sham_build)
    set(SHAM_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/third_party/sham/sham/sham)
	set(SHAM_OPTS
	    CFLAGS="${CMAKE_C_FLAGS}"
	    LDFLAGS="${CMAKE_SHARED_LINKER_FLAGS}")
	separate_arguments(SHAM_OPTS)
    if (${PROJECT_BINARY_DIR} STREQUAL ${PROJECT_SOURCE_DIR})
		add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/third_party/sham/libsham.a
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/third_party/sham
			COMMAND $(MAKE) ${SHAM_OPTS} clean
			COMMAND $(MAKE) ${SHAM_OPTS} static
			DEPENDS ${CMAKE_SOURCE_DIR}/CMakeCache.txt
		)
    else()
        add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/third_party/sham
            COMMAND ${CMAKE_COMMAND} -E make_directory ${PROJECT_BINARY_DIR}/third_party/sham
        )
		add_custom_command(OUTPUT ${PROJECT_BINARY_DIR}/third_party/sham/libsham.a
			WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/third_party/sham
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/third_party/sham ${PROJECT_BINARY_DIR}/third_party/sham
			COMMAND $(MAKE) ${SHAM_OPTS} clean
			COMMAND $(MAKE) ${SHAM_OPTS} static
            DEPENDS ${PROJECT_BINARY_DIR}/CMakeCache.txt ${PROJECT_BINARY_DIR}/third_party/sham
		)
    endif()
	add_custom_target(libsham ALL
		DEPENDS ${PROJECT_BINARY_DIR}/third_party/sham/libsham.a
	)
    message(STATUS "Use bundled Sham: ${PROJECT_SOURCE_DIR}/third_party/sham/")
    set (sham_lib "${PROJECT_BINARY_DIR}/third_party/sham/libsham.a")
    add_dependencies(build_bundled_libs libsham)
endmacro(sham_build)
