add_library(TrinexReflectorCore STATIC src/parser.cpp src/printer.cpp src/tokenizer.cpp)
target_compile_features(TrinexReflectorCore PRIVATE cxx_std_20)
target_include_directories(TrinexReflectorCore PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(TrinexReflector src/main.cpp)
target_compile_features(TrinexReflector PRIVATE cxx_std_20)
target_link_libraries(TrinexReflector PRIVATE TrinexReflectorCore)
target_include_directories(TrinexReflector PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(TrinexReflectorTests src/tests.cpp)
target_compile_features(TrinexReflectorTests PRIVATE cxx_std_20)
target_link_libraries(TrinexReflectorTests PRIVATE TrinexReflectorCore)
target_include_directories(TrinexReflectorTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)


