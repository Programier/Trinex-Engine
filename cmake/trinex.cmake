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
            get_property(modules TARGET ${library} PROPERTY INTERFACE_TRINEX_REFLECTION_MODULES)
            _trinex_add_reflection_modules(${target} TRINEX_REFLECTION_EXTERNAL_MODULES ${modules})

            if(scope STREQUAL "PUBLIC" OR scope STREQUAL "INTERFACE")
                _trinex_add_reflection_modules(${target} INTERFACE_TRINEX_REFLECTION_MODULES ${modules})
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

function(trinex_macro_headers target)
    cmake_parse_arguments(ARG "" "" "PUBLIC;PRIVATE;INTERFACE" ${ARGN})

    foreach(access IN ITEMS PUBLIC PRIVATE INTERFACE)
        foreach(file IN LISTS ARG_${access})
            get_filename_component(header "${file}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
            target_compile_options(${target} ${access} "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-imacros \"${header}\">")
        endforeach()
    endforeach()
endfunction()

function(trinex_compile_options target)
    target_compile_options(${target} ${ARGN})
endfunction()

function(trinex_target_properties target)
    set_target_properties(${target} ${ARGN})
endfunction()

function(trinex_sources target)
    cmake_parse_arguments(ARG "" "" "PUBLIC;PRIVATE" ${ARGN})

    list(APPEND ARG_PRIVATE ${ARG_UNPARSED_ARGUMENTS})

    foreach(scope IN ITEMS PUBLIC PRIVATE)
        set(sources)
        set(modules)

        foreach(file IN LISTS ARG_${scope})
            get_filename_component(ext "${file}" EXT)

            if(ext STREQUAL ".cpp")
                list(APPEND sources "${file}")
            elseif(ext STREQUAL ".cppm")
                list(APPEND modules "${file}")
            else()
                message(FATAL_ERROR "Unsupported source: ${file}")
            endif()
        endforeach()

        string(TOLOWER "${scope}" visibility)

        if(sources)
            target_sources(${target} ${scope} ${sources})
        endif()

        if(modules)
            target_sources(${target} ${scope} FILE_SET ${visibility}_modules TYPE CXX_MODULES FILES ${modules})

            set(reflection_modules)
            foreach(module IN LISTS modules)
                get_filename_component(module "${module}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
                list(APPEND reflection_modules "${module}")
            endforeach()

            _trinex_add_reflection_modules(${target} TRINEX_REFLECTION_MODULES ${reflection_modules})
            if(scope STREQUAL "PUBLIC")
                _trinex_add_reflection_modules(${target} INTERFACE_TRINEX_REFLECTION_MODULES ${reflection_modules})
            endif()
        endif()
    endforeach()
endfunction()

function(trinex_source_directories target)
    cmake_parse_arguments(ARG "RECURSIVE" "" "PUBLIC;PRIVATE" ${ARGN})

    list(APPEND ARG_PRIVATE ${ARG_UNPARSED_ARGUMENTS})

    foreach(scope IN ITEMS PUBLIC PRIVATE)
        set(sources)

        foreach(directory IN LISTS ARG_${scope})
            get_filename_component(directory "${directory}" ABSOLUTE BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

            if(NOT IS_DIRECTORY "${directory}")
                message(FATAL_ERROR "Source directory does not exist: ${directory}")
            endif()

            if(ARG_RECURSIVE)
                file(GLOB_RECURSE files CONFIGURE_DEPENDS "${directory}/*.cpp" "${directory}/*.cppm")
            else()
                file(GLOB files CONFIGURE_DEPENDS "${directory}/*.cpp" "${directory}/*.cppm")
            endif()

            list(APPEND sources ${files})
        endforeach()

        if(sources)
            list(REMOVE_DUPLICATES sources)
            trinex_sources(${target} ${scope} ${sources})
        endif()
    endforeach()
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

function(_trinex_add_reflection_modules target property)
    get_property(modules TARGET ${target} PROPERTY ${property})
    list(APPEND modules ${ARGN})
    list(REMOVE_DUPLICATES modules)
    set_property(TARGET ${target} PROPERTY ${property} "${modules}")
endfunction()

function(trinex_include_directories target)
	if(NOT TARGET ${target})
		message(FATAL_ERROR "Cannot add include directories: target '${target}' does not exist")
	endif()

	target_include_directories(${target} ${ARGN})
endfunction()

function(_trinex_configure_reflection target)
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

    get_property(source_modules TARGET ${target} PROPERTY TRINEX_REFLECTION_MODULES)
    get_property(external_modules TARGET ${target} PROPERTY TRINEX_REFLECTION_EXTERNAL_MODULES)

    set(reflection_sources)
    set(generated_modules)

    foreach(module IN LISTS source_modules)
        file(RELATIVE_PATH relative_path "${ARG_ROOT}" "${module}")
        string(REGEX REPLACE "\\.cppm$" ".generated.cpp" generated_source "${relative_path}")
        string(REGEX REPLACE "\\.cppm$" ".generated.cppm" generated_module "${relative_path}")
        list(APPEND reflection_sources "${ARG_OUTPUT}/src/${generated_source}")
        list(APPEND generated_modules "${ARG_OUTPUT}/modules/${generated_module}")
    endforeach()

    list(REMOVE_DUPLICATES reflection_sources)
    list(REMOVE_DUPLICATES generated_modules)

    file(MAKE_DIRECTORY "${ARG_OUTPUT}")
    set(manifest "${ARG_OUTPUT}/manifest.txt")
    set(manifest_template "${CMAKE_CURRENT_BINARY_DIR}/reflection-manifests/${target}.txt")
    set(manifest_content "--root\n${ARG_ROOT}\n--output\n${ARG_OUTPUT}\n--files\n")
    foreach(module IN LISTS source_modules)
        string(APPEND manifest_content "${module}\n")
    endforeach()
    string(APPEND manifest_content "--external\n")
    foreach(module IN LISTS external_modules)
        string(APPEND manifest_content "${module}\n")
    endforeach()
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/reflection-manifests")
    file(GENERATE OUTPUT "${manifest_template}" CONTENT "${manifest_content}")

    add_custom_command(
        OUTPUT "${manifest}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${ARG_OUTPUT}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${manifest_template}" "${manifest}"
        DEPENDS "${manifest_template}"
        COMMENT "Generating ${target} reflection manifest"
        VERBATIM
    )

    add_custom_command(
        OUTPUT "${ARG_OUTPUT}/reflection.bin"
        COMMAND $<TARGET_FILE:TrinexReflector> "@${manifest}"
        DEPENDS TrinexReflector "${manifest}" ${source_modules} ${external_modules}
        WORKING_DIRECTORY "${ARG_ROOT}"
        COMMENT "Generating ${target} reflection code"
        COMMAND_EXPAND_LISTS
        VERBATIM
    )

    add_custom_target(${target}Reflection DEPENDS "${ARG_OUTPUT}/reflection.bin")

    set_source_files_properties(${reflection_sources} ${generated_modules} PROPERTIES GENERATED TRUE)
    target_sources(${target} PRIVATE ${reflection_sources})
    if(generated_modules)
        target_sources(${target} PRIVATE FILE_SET reflection_modules TYPE CXX_MODULES FILES ${generated_modules})
    endif()
    add_dependencies(${target} ${target}Reflection)
endfunction()

function(trinex_library target)
    cmake_parse_arguments(ARG "" "API;REFLECTION_ROOT;REFLECTION_OUTPUT" "SOURCES" ${ARGN})

    if(NOT ARG_API)
        message(FATAL_ERROR "trinex_library '${target}' requires an API macro name")
    endif()

    add_library(${target} ${ARG_UNPARSED_ARGUMENTS} ${ARG_SOURCES})
    if(WIN32)
        target_compile_definitions(${target}
            PRIVATE "${ARG_API}=__declspec(dllexport)"
            INTERFACE "${ARG_API}=__declspec(dllimport)"
        )
    else()
        target_compile_definitions(${target}
            PRIVATE "${ARG_API}=__attribute__((visibility(\"default\")))"
            INTERFACE "${ARG_API}=__attribute__((visibility(\"default\")))"
        )
    endif()
    cmake_language(EVAL CODE "cmake_language(DEFER CALL _trinex_configure_reflection [=[${target}]=] ROOT [=[${ARG_REFLECTION_ROOT}]=] OUTPUT [=[${ARG_REFLECTION_OUTPUT}]=])")
endfunction()

function(trinex_executable target)
    cmake_parse_arguments(ARG "" "API;REFLECTION_ROOT;REFLECTION_OUTPUT" "SOURCES" ${ARGN})

    add_executable(${target} ${ARG_UNPARSED_ARGUMENTS} ${ARG_SOURCES})
    
    if(ARG_API)
        if(WIN32)
            target_compile_definitions(${target} PRIVATE "${ARG_API}=__declspec(dllexport)")
        else()
            target_compile_definitions(${target} PRIVATE "${ARG_API}=__attribute__((visibility(\"default\")))")
        endif()
    endif()
    
    cmake_language(EVAL CODE "cmake_language(DEFER CALL _trinex_configure_reflection [=[${target}]=] ROOT [=[${ARG_REFLECTION_ROOT}]=] OUTPUT [=[${ARG_REFLECTION_OUTPUT}]=])")
endfunction()
