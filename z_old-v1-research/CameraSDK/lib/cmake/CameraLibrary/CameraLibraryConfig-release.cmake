#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "CameraLibrary::CameraLibrary" for configuration "Release"
set_property(TARGET CameraLibrary::CameraLibrary APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(CameraLibrary::CameraLibrary PROPERTIES
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libCameraLibrary.so"
  IMPORTED_SONAME_RELEASE "libCameraLibrary.so"
  )

list(APPEND _cmake_import_check_targets CameraLibrary::CameraLibrary )
list(APPEND _cmake_import_check_files_for_CameraLibrary::CameraLibrary "${_IMPORT_PREFIX}/lib/libCameraLibrary.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
