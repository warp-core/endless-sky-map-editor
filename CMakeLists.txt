cmake_minimum_required(VERSION 3.16..3.25)

include(CMakeDependentOption)
if(UNIX AND NOT APPLE)
	option(ES_USE_VCPKG "Use vcpkg to get dependencies." OFF)
else()
	option(ES_USE_VCPKG "Use vcpkg to get dependencies." ON)
endif()
cmake_dependent_option(ES_CREATE_BUNDLE "Create a Bundle instead of an executable. Not suitable for development purposes." OFF APPLE OFF)

# Support Debug and Release configurations.
set(CMAKE_CONFIGURATION_TYPES "Debug" "Release" CACHE STRING "" FORCE)

# Use C++17 without any compiler specific extensions.
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# Use LTO for Release builds only.
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_DEBUG FALSE)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
# On Linux use relative RPATH.
set(CMAKE_BUILD_RPATH_USE_ORIGIN ON)

# Setup vcpkg.
if(ES_USE_VCPKG)
	set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
		CACHE STRING "Vcpkg toolchain file")
	# No need to bootstrap vcpkg if the toolchain file is explictly provided.
	if(CMAKE_TOOLCHAIN_FILE STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
		include(utils/vcpkg_bootstrap.cmake)
		x_vcpkg_bootstrap()
	endif()

	set(VCPKG_BOOTSTRAP_OPTIONS "-disableMetrics")
	set(VCPKG_INSTALL_OPTIONS "--no-print-usage")
	set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
	set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
else()
	# Avoid spurious warnings of unused variables.
	unset(VCPKG_HOST_TRIPLET)
	unset(VCPKG_TARGET_TRIPLET)
endif()

# Various helpful options for IDEs.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EndlessSkyEditor)
set(CMAKE_VS_JUST_MY_CODE_DEBUGGING ON)

project("Endless Sky Editor" VERSION 0.8.0
	DESCRIPTION "Map editor for the Endless Sky universe."
	HOMEPAGE_URL https://github.com/endless-sky/endless-sky-editor
	LANGUAGES CXX)

# Find the needed library dependencies.
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)
qt_standard_project_setup()

# Find the MinGW runtime DLLs.
if(MINGW AND WIN32)
	# On Windows copy the MinGW runtime DLLs to the output folder as well.
	# This is to avoid the situation where a user has other MinGW runtime DLLs
	# in their PATH that are incompatible with the MinGW used to compile ES.
	get_filename_component(PARENT_DIR "${CMAKE_CXX_COMPILER}" DIRECTORY)
	get_filename_component(MINGW_RUNTIME_DIR "${PARENT_DIR}" DIRECTORY)

	# MinGW doesn't have seh exceptions support for 32-bit Windows unfortunately,
	# and requires libgcc_s_dw2-1.dll instead of libgcc_s_seh-1.dll. There's no
	# perfect way to figure out which one to copy, so we simply copy both.
	# The executable will choose the correct DLL anyways.
	foreach(lib "stdc++-6" "winpthread-1" "gcc_s_seh-1" "gcc_s_dw2-1")
		file(GLOB_RECURSE FILE_PATH "${MINGW_RUNTIME_DIR}/lib${lib}.dll")
		if(FILE_PATH)
			list(APPEND MINGW_RUNTIME "${FILE_PATH}")
		endif()
	endforeach()
endif()

# Create game target.
if(APPLE AND ES_CREATE_BUNDLE)
	qt_add_executable(EndlessSkyEditor MACOSX_BUNDLE source/main.cpp)

	# Create the icns file, required for the bundle icon.
	add_custom_command(
		OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/resources/endless-sky.icns"
		COMMAND iconutil -c icns -o resources/endless-sky-editor.icns resources/endless-sky-editor.iconset
		DEPENDS resources/endless-sky-editor.iconset/icon_16x16.png resources/endless-sky-editor.iconset/icon_32x32.png
			resources/endless-sky-editor.iconset/icon_128x128.png
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" VERBATIM)

	# Now do the same to standalone files.
	foreach(file "license.txt" "resources/icons/endless-sky.icns")
		target_sources(EndlessSkyEditor PRIVATE ${file})
		set_source_files_properties(${file} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources/")
	endforeach()

	# Add plist to bundle.
	set_target_properties(EndlessSkyEditor PROPERTIES
		MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/resources/EndlessSkyEditor-Info.plist"
		XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME endless-sky-editor)
	set_target_properties(EndlessSkyEditor PROPERTIES OUTPUT_NAME "Endless Sky Editor")
elseif(WIN32)
    qt_add_executable(EndlessSkyEditor WIN32 source/main.cpp source/WinApp.rc)
	set_target_properties(EndlessSkyEditor PROPERTIES OUTPUT_NAME "Endless Sky Editor")
else()
	qt_add_executable(EndlessSkyEditor source/main.cpp)
	set_target_properties(EndlessSkyEditor PROPERTIES OUTPUT_NAME "endless-sky-editor")
endif()

# The 'mingw32' lib needs to be linked first.
if(MINGW)
	target_link_libraries(EndlessSkyEditor INTERFACE mingw32)
endif()

# Add the library target.
add_subdirectory(source)

# Link with the library dependencies.
target_link_libraries(EndlessSkyEditor PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

# Copy the MinGW runtime DLLs if necessary.
if(MINGW AND WIN32)
	foreach(FILE_PATH ${MINGW_RUNTIME})
		add_custom_command(TARGET EndlessSkyEditor POST_BUILD
			COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${FILE_PATH}" "$<TARGET_FILE_DIR:EndlessSkyEditor>"
			COMMAND_EXPAND_LISTS VERBATIM)

		# Add an install rule for this DLLs, so that it is also included when installing.
		install(FILES ${FILE_PATH} DESTINATION .)
	endforeach()
endif()

# When using the vcpkg built Qt, we need to specify the right paths to the Qt platform plugin for the binary to work.
if(ES_USE_VCPKG AND NOT ES_CREATE_BUNDLE)
	configure_file(resources/qt.conf.in ${CMAKE_BINARY_DIR}/qt.conf @ONLY)
	set(DEBUG_PREFIX "debug/")
	configure_file(resources/qt.conf.in ${CMAKE_BINARY_DIR}/qt-debug.conf @ONLY)
	unset(DEBUG_PREFIX)

	add_custom_command(TARGET EndlessSkyEditor POST_BUILD
		COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${CMAKE_BINARY_DIR}/qt$<$<CONFIG:Debug>:-debug>.conf "$<TARGET_FILE_DIR:EndlessSkyEditor>/qt.conf"
		VERBATIM)
endif()

# Installation configurations.
if(APPLE)
	install(TARGETS EndlessSkyEditor CONFIGURATIONS Release BUNDLE DESTINATION .)
elseif(WIN32)
	# Install the binary.
	install(TARGETS EndlessSkyEditor CONFIGURATIONS Release RUNTIME DESTINATION .)

	# The MinGW DLLs needed were already installed above, and vcpkg installs the library DLLs.

	# Install the resource files.
	install(FILES license.txt DESTINATION .)
elseif(UNIX)
	# Install the binary.
	install(TARGETS EndlessSkyEditor CONFIGURATIONS Release RUNTIME DESTINATION games)

	# Install the desktop file.
	install(FILES io.github.endless_sky.endless_sky_editor.desktop DESTINATION share/applications)

	# TODO: Install app center metadata.
	# install(FILES io.github.endless_sky.endless_sky_editor.appdata.xml DESTINATION share/metainfo)

	# Install icons, keeping track of all the paths.
	# Most Ubuntu apps supply 16, 22, 24, 32, 48, and 256, and sometimes others.
	foreach(size "16x16" "32x32" "256x256")
		install(FILES "resources/icons/icon_${size}.png" DESTINATION "share/icons/hicolor/${size}/apps"
			RENAME endless-sky-editor.png)
	endforeach()

	# Gzip and install man file.
	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/endless-sky-editor.6.gz
		COMMAND gzip -c resources/endless-sky-editor.6 > ${CMAKE_CURRENT_BINARY_DIR}/endless-sky-editor.6.gz
		DEPENDS resources/endless-sky-editor.6
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		VERBATIM)
	target_sources(EndlessSkyEditor PRIVATE endless-sky-editor.6.gz)
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/endless-sky-editor.6.gz DESTINATION share/man/man6)

	# Install the resource files.
	install(FILES license.txt DESTINATION share/doc/endless-sky-editor)
endif()
