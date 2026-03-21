add_library(opentrackir
  src/tir5_protocol.c
  src/tir5_frame.c
  src/tir5_device.c
)
target_include_directories(opentrackir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(opentrackir PUBLIC opentrackir_libusb)

if(OPENTRACKIR_BUILD_TESTS)
  add_executable(test_tir5 tests/test_tir5.c)
  target_link_libraries(test_tir5 PRIVATE opentrackir)
  add_test(NAME test_tir5 COMMAND test_tir5)
endif()

add_executable(opentrackir_stream_dump examples/stream_dump.c)
target_link_libraries(opentrackir_stream_dump PRIVATE opentrackir)
