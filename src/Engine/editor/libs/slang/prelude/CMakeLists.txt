#
# Use the slang-embed tool to generate a source file for each prelude header.
# Construct a library called 'prelude' to be linked with by slang
#

glob_append(SLANG_PRELUDE_HEADERS "*-prelude.h")

if(SLANG_USE_PREGENERATED_HEADERS)
    set(SLANG_PRELUDE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/../pregenerated/prelude/slang-cpp-host-prelude.h.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/../pregenerated/prelude/slang-cpp-prelude.h.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/../pregenerated/prelude/slang-cuda-prelude.h.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/../pregenerated/prelude/slang-hlsl-prelude.h.cpp
                             ${CMAKE_CURRENT_SOURCE_DIR}/../pregenerated/prelude/slang-torch-prelude.h.cpp)
else()
    set(SLANG_PRELUDE_SOURCE)
    foreach(input ${SLANG_PRELUDE_HEADERS})
        get_filename_component(input_name ${input} NAME)
        set(output "${CMAKE_CURRENT_BINARY_DIR}/${input_name}.cpp")
        add_custom_command(
            OUTPUT ${output}
            COMMAND slang-embed "${input}" ${output}
            DEPENDS ${input} slang-embed
            VERBATIM
        )
        list(APPEND SLANG_PRELUDE_SOURCE ${output})
    endforeach()
endif()

slang_add_target(
    .
    OBJECT
    EXPLICIT_SOURCE ${SLANG_PRELUDE_SOURCE}
    EXCLUDE_FROM_ALL
    TARGET_NAME prelude
    INCLUDE_DIRECTORIES_PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PUBLIC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/slang*.h
    # It's an object library, so the install step only installs the headers
    INSTALL
    FOLDER generated
)

