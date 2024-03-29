cmake_minimum_required(VERSION 3.18.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Do not enable compiler specific extensions, for eg on GCC use -std=c++1z (=c++17) and not -std=gnu++17
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_VERBOSE_MAKEFILE TRUE)


# Use ccache is available, has to be before "project()"
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
  # Support Unix Makefiles and Ninja
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CCACHE_PROGRAM}")
endif()


project(test_swig VERSION 0.0.1)

# Set a default build type if none was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to 'Release' as none was specified.")
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()

###############################################################################
#                                  N I N J A                                  #
###############################################################################

# Xcode/Ninja generators undefined MAKE
if(CMAKE_GENERATOR MATCHES "Make")
  set(MAKE "$(MAKE)")
else()
  set(MAKE make)
endif()

# Ninja support: has to be atop for it to take effect before anything else is done
# Add Color Output if Using Ninja
macro(AddCXXFlagIfSupported flag test)
  CHECK_CXX_COMPILER_FLAG(${flag} ${test})
  if(${${test}})
    message("adding ${flag}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
  endif()
endmacro()

if("Ninja" STREQUAL ${CMAKE_GENERATOR})
  # Clang
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    AddCXXFlagIfSupported(-fcolor-diagnostics COMPILER_SUPPORTS_fcolor-diagnostics)
  endif()

  # g++
  if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # For some reason it doesn't say its supported, but it works...
    # AddCXXFlagIfSupported(-fdiagnostics-color COMPILER_SUPPORTS_fdiagnostics-color)
    message(STATUS "Ninja: Forcing -fdiagnostics-color=always")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()


# Search first in the binary dir, where conan will install finders, then
# search for modules in the root dir to override cmake ones
# Start with ROOT, then PROJECT_BINARY_DIR
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR} ${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/CMake")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Add to include path

# Project source directory
include_directories("${PROJECT_SOURCE_DIR}")

###############################################################################
#                         C M A K E    C O N T R O L                          #
###############################################################################

# High level project configuration
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/Products")

set(LIBRARY_SEARCH_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Release" "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/Debug")

# Search first in the binary dir, where conan will install finders, then
# search for modules in the root dir to override cmake ones
list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR} ${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/CMake")

#######################################################################
#           R E G U L A R    O U P U T    A N D    L I B S            #
#######################################################################



add_library(printname SHARED
  printname.hpp
  printname.cpp
)

add_library(mylib STATIC
  Person.hpp
  Person.cpp
)
target_link_libraries(printname PRIVATE mylib)

# API layer
add_library(api SHARED
  api.hpp
  api.cpp
)
target_link_libraries(api PUBLIC mylib)

set(Python_USE_STATIC_LIBS OFF)
find_package(Python 3.8 EXACT COMPONENTS Interpreter Development REQUIRED)
message("Python_LIBRARIES=${Python_LIBRARIES}")

get_filename_component(RESOLVED_PYTHON_LIBRARY "${Python_LIBRARIES}" REALPATH)
message("RESOLVED_PYTHON_LIBRARY=${RESOLVED_PYTHON_LIBRARY}")
if(WIN32)

  # In case you have both release and debug Python libraries on your system, Python_LIBRARIES might be "optimized;C:/.../python38.lib;debug;C:/.../python38_d.lib"
  # so it can't be used directly, we could use a generator expression to find the $<TARGET_FILE:Python::Python> library used and that'd point directly
  # to the DLL. But it won't work nicely for the install(code ...) command below... so we do a hack, get the optimized one always...
  list(LENGTH Python_LIBRARIES _LEN)
  if (_LEN GREATER 1)
    set (_Python_DOING_RELEASE FALSE)
    foreach(_currentArg ${Python_LIBRARIES})
      if ("x${_currentArg}" STREQUAL "xoptimized")
        set(_Python_DOING_RELEASE TRUE)
      elseif(_Python_DOING_RELEASE)
        get_filename_component(RESOLVED_PYTHON_LIBRARY "${_currentArg}" REALPATH)
        break()
      endif()
    endforeach()
  # else()
    # No-op, already done above
  endif()

  install(IMPORTED_RUNTIME_ARTIFACTS Python::Python LIBRARY DESTINATION lib)

else()
  get_target_property(PYTHON_LIB_TYPE Python::Python TYPE)
  message("PYTHON_LIB_TYPE=${PYTHON_LIB_TYPE}")
  if(NOT PYTHON_LIB_TYPE STREQUAL STATIC_LIBRARY)
    #  message(FATAL_ERROR "We expect to find a static libpython on Unix")
  endif()
  # Does NOT follow symlinks. Works fine on windows and mac, but on windows it ships a libpython3.8.so which is just a symlink
  install(IMPORTED_RUNTIME_ARTIFACTS Python::Python LIBRARY DESTINATION lib)
  install(FILES ${RESOLVED_PYTHON_LIBRARY} DESTINATION lib)

endif()

target_link_libraries(api PRIVATE Python::Python)

# Executable
add_executable(Test
  main.cpp
)

if(APPLE)
  # This works:
  set_target_properties(Test PROPERTIES
    #  # MACOSX_RPATH ON
    INSTALL_RPATH "@executable_path;@executable_path/../lib/"
    #  INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib" -> points to /usr/local/lib when I don't change CMAKE_INSTALL_PREFIX
    #  # INSTALL_RPATH_USE_LINK_PATH TRUE
  )
  set_target_properties(api PROPERTIES
    #  # MACOSX_RPATH ON
    INSTALL_RPATH "@loader_path"
  )
  install(CODE [[
    execute_process(COMMAND "install_name_tool" -change "$<TARGET_FILE:Python::Python>" "@loader_path/$<TARGET_FILE_NAME:Python::Python>" "$<TARGET_FILE:api>")
  ]])
elseif(UNIX)
  set_target_properties(Test PROPERTIES
    INSTALL_RPATH "$ORIGIN:$ORIGIN/../lib/")

  set_target_properties(api PROPERTIES
    INSTALL_RPATH "$ORIGIN")
endif()

target_link_libraries(Test PRIVATE api printname)

install(TARGETS Test DESTINATION bin)
install(TARGETS printname DESTINATION lib)
install(TARGETS api DESTINATION lib)

set(CPACK_GENERATOR "TGZ")
include(CPack)
