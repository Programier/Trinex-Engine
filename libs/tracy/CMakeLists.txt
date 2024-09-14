set(TRACY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/TracyClient.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyDxt1.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracySysPower.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracySysTime.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyAlloc.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyOverride.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyCallstack.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracySysTrace.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/tracy_rpmalloc.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyProfiler.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/client/TracyKCore.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/elf.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/alloc.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/sort.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/mmapio.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/fileline.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/dwarf.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/state.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/posix.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/libbacktrace/macho.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/common/TracySystem.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/common/TracySocket.cpp
																	 ${CMAKE_CURRENT_SOURCE_DIR}/common/TracyStackFrames.cpp)


add_trinex_sources(${TRACY_SOURCES})
add_trinex_private_include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_trinex_public_definitions("TRACY_ENABLE")
add_trinex_private_definitions("TRACY_EXPORTS")
