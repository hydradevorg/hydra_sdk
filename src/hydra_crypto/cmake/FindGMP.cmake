# FindGMP.cmake
# Try to find the GMP library
# Once done this will define
#  GMP_FOUND - System has GMP
#  GMP_INCLUDE_DIRS - The GMP include directories
#  GMP_LIBRARIES - The libraries needed to use GMP
#  GMP_DEFINITIONS - Compiler switches required for using GMP

find_path(GMP_INCLUDE_DIR NAMES gmp.h
          PATHS /usr/include /usr/local/include ${CMAKE_SOURCE_DIR}/include)

find_library(GMP_LIBRARY NAMES gmp
             PATHS /usr/lib /usr/local/lib ${CMAKE_SOURCE_DIR}/lib)

include(FindPackageHandleStandardArgs)
# handle the QUIETLY and REQUIRED arguments and set GMP_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args(GMP DEFAULT_MSG
                                  GMP_LIBRARY GMP_INCLUDE_DIR)

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY)

if(GMP_FOUND)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
  
  # Create an imported target for GMP
  if(NOT TARGET GMP::GMP)
    add_library(GMP::GMP UNKNOWN IMPORTED)
    set_target_properties(GMP::GMP PROPERTIES
      IMPORTED_LOCATION "${GMP_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${GMP_INCLUDE_DIR}")
  endif()
endif()
