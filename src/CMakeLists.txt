file(GLOB_RECURSE HEADER_FILES
	"*.hpp"
	"*.h")

file(GLOB_RECURSE SOURCE_FILES
	"*.cpp"
	"*.c")

file(GLOB_RECURSE RESOURCES_FILES
	"*.rc"
	"*.ico")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Source Files" FILES ${SOURCE_FILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Resources Files" FILES ${RESOURCES_FILES})

if(IOS)
	# Optional: bundle a local copy of the game data into the app (it is not redistributable).
	# Otherwise copy "Doom 2 RPG.ipa" into the app's Documents folder via the Files app / Finder.
	set(DOOM2RPG_IPA "" CACHE FILEPATH "Path to 'Doom 2 RPG.ipa' to bundle into the app")
	set(DOOM2RPG_BUNDLE_ID "com.gec.doom2rpg" CACHE STRING "iOS bundle identifier")
	set(IOS_RESOURCES "${CMAKE_SOURCE_DIR}/iOS/LaunchScreen.storyboard")
	if(DOOM2RPG_IPA)
		list(APPEND IOS_RESOURCES "${DOOM2RPG_IPA}")
	endif()

	add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${HEADER_FILES} ${SOURCE_FILES} ${IOS_RESOURCES}
		"${CMAKE_SOURCE_DIR}/iOS/SceneDelegate.m")
	set_source_files_properties("${CMAKE_SOURCE_DIR}/iOS/SceneDelegate.m" PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
	set_source_files_properties(${IOS_RESOURCES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)

	set_target_properties(${PROJECT_NAME} PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/iOS/Info.plist.in"
		MACOSX_BUNDLE_GUI_IDENTIFIER "${DOOM2RPG_BUNDLE_ID}"
		MACOSX_BUNDLE_BUNDLE_NAME "Doom II RPG"
		MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
		MACOSX_BUNDLE_BUNDLE_VERSION "1"
		XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${DOOM2RPG_BUNDLE_ID}"
		XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
		XCODE_ATTRIBUTE_CODE_SIGN_STYLE "Automatic"
	)

	target_compile_definitions(${PROJECT_NAME} PRIVATE GLES_SILENCE_DEPRECATION)
	# The sources include <al.h>/<alc.h> directly
	target_compile_options(${PROJECT_NAME} PRIVATE "-I$(SDKROOT)/System/Library/Frameworks/OpenAL.framework/Headers")

	target_link_libraries(${PROJECT_NAME}
		SDL2::SDL2main
		SDL2::SDL2-static
		z
		"-framework OpenAL"
		"-framework OpenGLES"
		"-framework UIKit"
		"-framework Foundation"
		${HASH_LIBRARY_TGT_NAME})
else()
	add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCES_FILES})

	target_link_libraries(${PROJECT_NAME}
		SDL2::SDL2
		ZLIB::ZLIB
		OpenAL::OpenAL
		OpenGL::GL
		${HASH_LIBRARY_TGT_NAME})
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
