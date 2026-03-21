if(NOT OpenCV_FOUND)
  message(STATUS "OpenCV not found; skipping opentrackir_preview")
  return()
endif()

add_executable(opentrackir_preview opencv_preview/main.cpp)
target_include_directories(opentrackir_preview PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(opentrackir_preview PRIVATE opentrackir ${OpenCV_LIBS})
