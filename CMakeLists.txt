cmake_minimum_required(VERSION 3.20)
project(FieldKitLauncher LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

option(FIELDKIT_FETCH_DEPENDENCIES "Download GLFW and FreeType when they are not installed" ON)

find_package(glfw3 3.3 QUIET)
find_package(Freetype QUIET)

if((NOT TARGET glfw OR NOT TARGET Freetype::Freetype) AND FIELDKIT_FETCH_DEPENDENCIES)
	include(FetchContent)

	if(NOT TARGET glfw)
		set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
		set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
		FetchContent_Declare(
			glfw
			GIT_REPOSITORY https://github.com/glfw/glfw.git
			GIT_TAG 3.4
			GIT_SHALLOW TRUE
		)
		FetchContent_MakeAvailable(glfw)
	endif()

	if(NOT TARGET Freetype::Freetype)
		set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
		set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
		set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
		set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
		FetchContent_Declare(
			freetype
			GIT_REPOSITORY https://github.com/freetype/freetype.git
			GIT_TAG VER-2-13-3
			GIT_SHALLOW TRUE
		)
		FetchContent_MakeAvailable(freetype)
	endif()

	if(TARGET freetype-interface AND NOT TARGET Freetype::Freetype)
		add_library(Freetype::Freetype ALIAS freetype-interface)
	endif()
endif()

if(NOT TARGET glfw)
	message(FATAL_ERROR "GLFW was not found. Install GLFW or configure with -DFIELDKIT_FETCH_DEPENDENCIES=ON.")
endif()
if(NOT TARGET Freetype::Freetype)
	message(FATAL_ERROR "FreeType was not found. Install FreeType or configure with -DFIELDKIT_FETCH_DEPENDENCIES=ON.")
endif()

add_executable(field_kit main.cpp glad.c)
target_include_directories(field_kit PRIVATE include glm)
target_compile_definitions(field_kit PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(field_kit PRIVATE glfw Freetype::Freetype Threads::Threads ${CMAKE_DL_LIBS})

option(FIELDKIT_AGGRESSIVE_OPTIMIZATIONS "Enable aggressive release optimizations for the target PC" ON)

if(FIELDKIT_AGGRESSIVE_OPTIMIZATIONS)
	include(CheckIPOSupported)
	check_ipo_supported(RESULT FIELDKIT_IPO_SUPPORTED OUTPUT FIELDKIT_IPO_ERROR LANGUAGES C CXX)
	if(FIELDKIT_IPO_SUPPORTED)
		set_property(TARGET field_kit PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
	else()
		message(STATUS "Release IPO is unavailable: ${FIELDKIT_IPO_ERROR}")
	endif()

	if(MSVC)
		target_compile_options(field_kit PRIVATE
			$<$<CONFIG:Release>:/O2>
			$<$<CONFIG:Release>:/Oi>
			$<$<CONFIG:Release>:/Ot>
			$<$<CONFIG:Release>:/Gy>
			$<$<CONFIG:Release>:/Gw>
		)
		target_link_options(field_kit PRIVATE
			$<$<CONFIG:Release>:/OPT:REF>
			$<$<CONFIG:Release>:/OPT:ICF>
		)
		if(CMAKE_SIZEOF_VOID_P EQUAL 4)
			target_compile_options(field_kit PRIVATE $<$<CONFIG:Release>:/arch:SSE2>)
		endif()
	elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(field_kit PRIVATE
			$<$<CONFIG:Release>:-O3>
			$<$<CONFIG:Release>:-ffunction-sections>
			$<$<CONFIG:Release>:-fdata-sections>
		)
		if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64|i[3-6]86)$")
			target_compile_options(field_kit PRIVATE
				$<$<CONFIG:Release>:-march=silvermont>
				$<$<CONFIG:Release>:-mtune=silvermont>
			)
		endif()
		if(WIN32 OR (UNIX AND NOT APPLE))
			target_link_options(field_kit PRIVATE $<$<CONFIG:Release>:-Wl,--gc-sections>)
		endif()
	endif()
endif()