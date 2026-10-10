include(CMakeParseArguments)

function(trinex_link_libraries target)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot link libraries: target '${target}' does not exist")
    endif()

    target_link_libraries(${target} ${ARGN})

    set(scope PRIVATE)
    set(keyword)
    foreach(library ${ARGN})
        if(library STREQUAL "PUBLIC" OR library STREQUAL "PRIVATE" OR library STREQUAL "INTERFACE")
            set(scope ${library})
        elseif(library STREQUAL "debug" OR library STREQUAL "optimized" OR library STREQUAL "general")
            set(keyword ${library})
        elseif(keyword)
            set(keyword)
        elseif(TARGET ${library})
            get_property(reflection_directories TARGET ${library} PROPERTY INTERFACE_TRINEX_REFLECTION_DIRECTORIES)
            get_property(include_directories TARGET ${library} PROPERTY INTERFACE_TRINEX_REFLECTION_INCLUDE_DIRECTORIES)

            _trinex_add_reflection_directories(${target} TRINEX_REFLECTION_INCLUDE_DIRS
                ${reflection_directories} ${include_directories})

            if(scope STREQUAL "PUBLIC" OR scope STREQUAL "INTERFACE")
                _trinex_add_reflection_directories(${target} INTERFACE_TRINEX_REFLECTION_INCLUDE_DIRECTORIES
                    ${reflection_directories} ${include_directories})
            endif()
        endif()
    endforeach()
endfunction()

function(trinex_link_directories target)
    target_link_directories(${target} ${ARGN})
endfunction()

function(trinex_compile_definitions target)
    target_compile_definitions(${target} ${ARGN})
endfunction()

function(trinex_compile_options target)
    target_compile_options(${target} ${ARGN})
endfunction()

function(trinex_sources target)
    target_sources(${target} ${ARGN})
endfunction()

function(trinex_set_properties target)
    set_target_properties(${target} ${ARGN})
endfunction()

function(trinex_set_property target)
    set_property(TARGET ${target} ${ARGN})
endfunction()

function(trinex_get_property output target property)
    get_target_property(value ${target} ${property})
    set(${output} "${value}" PARENT_SCOPE)
endfunction()

function(trinex_install)
    install(${ARGN})
endfunction()

function(_trinex_add_reflection_directories target property)
    get_property(directories TARGET ${target} PROPERTY ${property})
    list(APPEND directories ${ARGN})
    list(REMOVE_DUPLICATES directories)
    set_property(TARGET ${target} PROPERTY ${property} "${directories}")
endfunction()

function(_trinex_include_directories target property)
    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot add include directories: target '${target}' does not exist")
    endif()

    cmake_parse_arguments(ARG "BEFORE;AFTER" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})

    if(ARG_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown include-directory arguments: ${ARG_UNPARSED_ARGUMENTS}")
    endif()

    if(ARG_BEFORE AND ARG_AFTER)
        message(FATAL_ERROR "Include directories cannot be both BEFORE and AFTER")
    endif()

    if(NOT ARG_PUBLIC AND NOT ARG_PRIVATE AND NOT ARG_INTERFACE)
        message(FATAL_ERROR "At least one include-directory visibility is required")
    endif()

    set(arguments ${target})
    if(ARG_BEFORE)
        list(APPEND arguments BEFORE)
    elseif(ARG_AFTER)
        list(APPEND arguments AFTER)
    endif()

    foreach(scope PUBLIC PRIVATE INTERFACE)
        if(ARG_${scope})
            list(APPEND arguments ${scope} ${ARG_${scope}})
            _trinex_add_reflection_directories(${target} ${property} ${ARG_${scope}})

            if(scope STREQUAL "PUBLIC" OR scope STREQUAL "INTERFACE")
                _trinex_add_reflection_directories(${target} INTERFACE_${property} ${ARG_${scope}})
            endif()
        endif()
    endforeach()

    target_include_directories(${arguments})
endfunction()

function(trinex_include_directories target)
	if(NOT TARGET ${target})
		message(FATAL_ERROR "Cannot add include directories: target '${target}' does not exist")
	endif()

	target_include_directories(${target} ${ARGN})
endfunction()

function(trinex_reflection_directories target)
    _trinex_include_directories(${target} TRINEX_REFLECTION_DIRECTORIES ${ARGN})
endfunction()

function(trinex_reflect target)
    cmake_parse_arguments(ARG "" "ROOT;OUTPUT" "" ${ARGN})

    if(NOT TARGET ${target})
        message(FATAL_ERROR "Cannot add reflection: target '${target}' does not exist")
    endif()

    if(NOT TARGET TrinexReflector)
        message(FATAL_ERROR "Cannot add reflection for '${target}': TrinexReflector is unavailable")
    endif()

    if(NOT ARG_ROOT)
        set(ARG_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
    endif()

    if(NOT ARG_OUTPUT)
        set(ARG_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/reflection/${target}")
    endif()

    add_custom_target(${target}Reflection
        COMMAND $<TARGET_FILE:TrinexReflector>
            --root "${ARG_ROOT}"
            --output "${ARG_OUTPUT}"
            --scan-dirs $<TARGET_PROPERTY:${target},TRINEX_REFLECTION_DIRECTORIES>
            --include-dirs $<TARGET_PROPERTY:${target},TRINEX_REFLECTION_INCLUDE_DIRS>
        DEPENDS TrinexReflector
        WORKING_DIRECTORY "${ARG_ROOT}"
        COMMENT "Generating ${target} reflection code"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    add_dependencies(${target} ${target}Reflection)
    target_include_directories(${target} PUBLIC "${ARG_OUTPUT}/include")
endfunction()

function(trinex_library target)
    cmake_parse_arguments(ARG "NO_REFLECTION" "REFLECTION_ROOT;REFLECTION_OUTPUT"
                          "SOURCES" ${ARGN})

    add_library(${target} ${ARG_UNPARSED_ARGUMENTS} ${ARG_SOURCES})

    if(NOT ARG_NO_REFLECTION)
        trinex_reflect(${target}
            ROOT "${ARG_REFLECTION_ROOT}"
            OUTPUT "${ARG_REFLECTION_OUTPUT}"
        )
    endif()
endfunction()

function(trinex_executable target)
    cmake_parse_arguments(ARG "NO_REFLECTION" "REFLECTION_ROOT;REFLECTION_OUTPUT"
                          "SOURCES" ${ARGN})

    add_executable(${target} ${ARG_UNPARSED_ARGUMENTS} ${ARG_SOURCES})

    if(NOT ARG_NO_REFLECTION)
        trinex_reflect(${target}
            ROOT "${ARG_REFLECTION_ROOT}"
            OUTPUT "${ARG_REFLECTION_OUTPUT}"
        )
    endif()
endfunction()
