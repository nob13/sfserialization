cmake_minimum_required (VERSION 2.6)
project (sfserialization)
enable_testing ()

# set (CMAKE_VERBOSE_MAKEFILE ON)


if (NOT CMAKE_BUILD_TYPE)
	set (CMAKE_BUILD_TYPE "DEBUG")
endif()

if (WIN32)
	add_definitions("-DWIN32")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS") # otherwise windows doesn't like fopen etc.
endif()

if (NOT DEFINED WIN32)
	# Warnings
	set(CMAKE_CXX_FLAGS "-Wall")
endif()

message (STATUS "CXX Flags:      ${CMAKE_CXX_FLAGS}")
message (STATUS "Install Prefix: ${CMAKE_INSTALL_PREFIX}") 

set (LIBS "")

# Boost dependency
message (STATUS " - boost")
	set (Boost_USE_STATIC_LIBS TRUE)
	
	if (NOT Boost_FOUND) # could also be inserted via -DBoost_FOUND TRUE etc.
		find_package (Boost 1.40.0)
	endif()
	if (Boost_FOUND)
		message (STATUS "Boost Dir: ${Boost_INCLUDE_DIRS}")
		set (LIBS ${LIBS} ${Boost_LIBRARIES})
		message (STATUS "Boost Libs: ${Boost_LIBRARIES}")
		message (STATUS "Boost Ver: ${Boost_VERSION}")
		include_directories (${Boost_INCLUDE_DIRS})
	else()
		message (FATAL_ERROR "Boost not found")
	endif()


# sf serialization library
add_subdirectory (sfserialization)
set (LIBS ${LIBS} "sfserialization")
include_directories (".")

# Main Library & Executable
add_subdirectory (sfautoreflect)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/sfautoreflect)
set (LIBS "reflect" ${LIBS})

# Test Cases
add_subdirectory (testcases)

# Uninstall Target
# (HAS BUGS; Does not delete the directories itself)
# See http://www.cmake.org/Wiki/CMake_FAQ#Can_I_do_.22make_uninstall.22_with_CMake.3F
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
