cmake_minimum_required(VERSION 3.22)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project(DoomIIRPG)

add_compile_options(
    -fsigned-char         # Force char to be signed
)

if(MSVC)
	add_definitions("/D_CRT_SECURE_NO_WARNINGS")
endif()

if(UNIX)
	add_definitions("-Wno-write-strings")
endif()

if(IOS)
	# iOS: build SDL2 from source as a static library (works for device and simulator)
	include(FetchContent)
	set(SDL_SHARED OFF CACHE BOOL "" FORCE)
	set(SDL_STATIC ON CACHE BOOL "" FORCE)
	set(SDL_TEST OFF CACHE BOOL "" FORCE)
	FetchContent_Declare(SDL2
		GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
		GIT_TAG release-2.32.10
		GIT_SHALLOW TRUE)
	FetchContent_MakeAvailable(SDL2)
	# zlib comes with the iOS SDK (linked as -lz so it works for device and simulator)
else()
	find_package(SDL2 REQUIRED)
	find_package(ZLIB REQUIRED)
	find_package(OpenAL REQUIRED)
	find_package(OpenGL REQUIRED)
endif()


# Global identifiers for each project/target
set(HASH_LIBRARY_TGT_NAME           Hash-Library)
add_subdirectory("${PROJECT_SOURCE_DIR}/src")
add_subdirectory("${PROJECT_SOURCE_DIR}/third_party_libs/hash-library")