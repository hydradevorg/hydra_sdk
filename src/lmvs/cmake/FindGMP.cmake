# FindGMP.cmake - Find the GMP library
# This module defines
#  GMP_FOUND - System has GMP
#  GMP_INCLUDE_DIRS - The GMP include directories
#  GMP_LIBRARIES - The libraries needed to use GMP

# Try to find GMP using the provided paths first
find_path(GMP_INCLUDE_DIR NAMES gmp.h
          PATHS ${GMP_INCLUDE_DIR} ${CMAKE_PREFIX_PATH}/include
          NO_DEFAULT_PATH)

find_library(GMP_LIBRARY NAMES gmp
             PATHS ${GMP_LIBRARY} ${CMAKE_PREFIX_PATH}/lib
             NO_DEFAULT_PATH)

# If not found, try the system paths
if(NOT GMP_INCLUDE_DIR)
  find_path(GMP_INCLUDE_DIR NAMES gmp.h)
endif()

if(NOT GMP_LIBRARY)
  find_library(GMP_LIBRARY NAMES gmp)
endif()

# Handle the QUIETLY and REQUIRED arguments and set GMP_FOUND to TRUE
# if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GMP DEFAULT_MSG
                                  GMP_LIBRARY GMP_INCLUDE_DIR)

if(GMP_FOUND)
  set(GMP_LIBRARIES ${GMP_LIBRARY})
  set(GMP_INCLUDE_DIRS ${GMP_INCLUDE_DIR})
endif()

mark_as_advanced(GMP_INCLUDE_DIR GMP_LIBRARY)
