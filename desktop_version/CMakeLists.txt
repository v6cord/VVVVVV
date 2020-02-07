# CMake File for VVVVVV
# Written by Ethan "flibitijibibo" Lee

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

if(SWITCH)
    CMAKE_POLICY(SET CMP0058 OLD)
endif()

if(CMAKE_TOOLCHAIN_FILE)
    mark_as_advanced(CMAKE_TOOLCHAIN_FILE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake Options
OPTION(ENABLE_WARNINGS "Enable compilation warnings" ON)
OPTION(ENABLE_WERROR "Treat compilation warnings as errors" OFF)
OPTION(ENABLE_PROFILE "Enable profiler" OFF)

# Architecture Flags
IF(APPLE)
	# Wow, Apple is a huge jerk these days huh?
	SET(OSX_10_9_SDK_PATH /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk)
	IF(NOT CMAKE_OSX_SYSROOT)
		IF(IS_DIRECTORY ${OSX_10_9_SDK_PATH})
			SET(CMAKE_OSX_SYSROOT ${OSX_10_9_SDK_PATH})
		ELSE()
			MESSAGE(WARNING "CMAKE_OSX_SYSROOT not set and macOS 10.9 SDK not found! Using default one.")
		ENDIF()
	ENDIF()
	SET(CMAKE_OSX_DEPLOYMENT_TARGET 10.9)
	LINK_DIRECTORIES(/usr/local/lib)
	ADD_COMPILE_OPTIONS(-Werror=partial-availability)
ENDIF()

if(ENABLE_PROFILE)
    ADD_COMPILE_OPTIONS(-finstrument-functions)
endif()

PROJECT(VVVVVV-CE)

find_package(Git)

SET(GITREV_BARE_FILE git-rev.h)
SET(GITREV_BARE_TMP git-rev-tmp.h)
SET(GITREV_FILE ${CMAKE_BINARY_DIR}/${GITREV_BARE_FILE})
SET(GITREV_TMP ${CMAKE_BINARY_DIR}/${GITREV_BARE_TMP})

ADD_CUSTOM_COMMAND(
    OUTPUT ${GITREV_TMP}
    COMMAND ${CMAKE_COMMAND} -E echo_append "#define GIT_BRANCH_RAW " > ${GITREV_TMP}
    COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD >> ${GITREV_TMP}
    COMMAND ${CMAKE_COMMAND} -E echo_append "#define GIT_HASH_RAW " >> ${GITREV_TMP}
    COMMAND ${GIT_EXECUTABLE} describe --always --dirty --abbrev=40 --match="NoTagWithThisName" >> ${GITREV_TMP}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GITREV_TMP} ${GITREV_FILE}
    COMMAND ${CMAKE_COMMAND} -E remove ${GITREV_TMP}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)

IF(APPLE)
	MESSAGE(STATUS "Using macOS SDK at ${CMAKE_OSX_SYSROOT}")
ENDIF()

# Compiler Flags
ADD_DEFINITIONS(-DPHYSFS_SUPPORTS_DEFAULT=0 -DPHYSFS_SUPPORTS_ZIP=1)

if (MINGW OR MSYS)
    add_definitions(-D__USE_MINGW_ANSI_STDIO=1)
endif()

# RPATH
IF(NOT WIN32)
	IF(APPLE)
		SET(BIN_LIBROOT "osx")
		SET(BIN_RPATH "@executable_path/osx")
	ELSEIF(CMAKE_SIZEOF_VOID_P MATCHES "8")
		SET(BIN_LIBROOT "lib64")
		SET(BIN_RPATH "\$ORIGIN/lib64")
	ELSE()
		SET(BIN_LIBROOT "lib")
		SET(BIN_RPATH "\$ORIGIN/lib")
	ENDIF()
	SET(CMAKE_SKIP_BUILD_RPATH TRUE)
	SET(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	SET(CMAKE_INSTALL_RPATH ${BIN_RPATH})
	SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
ENDIF()

# Creates C resources file from files in given directory
function(create_resource bin output)
    # Create empty output file
    file(WRITE ${output} "")
    # Get short filename
    string(REGEX MATCH "([^/]+)$" filename ${bin})
    # Replace filename spaces & extension separator for C compatibility
    string(REGEX REPLACE "\\.| |-" "_" filename ${filename})
    # Read hex data from file
    file(READ ${bin} filedata HEX)
    # Convert hex data for C compatibility
    string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," filedata ${filedata})
    # Append data to output file
    file(APPEND ${output} "const unsigned char ${filename}[] = {${filedata}};\nconst unsigned ${filename}_size = sizeof(${filename});\n")
endfunction()

file(GLOB_RECURSE data RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}/data" data/*)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar "cfv" "${CMAKE_BINARY_DIR}/vce.zip" --format=zip ${data} WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/data")
create_resource(${CMAKE_BINARY_DIR}/vce.zip ${CMAKE_BINARY_DIR}/vce.zip.c)

# Include Directories
INCLUDE_DIRECTORIES(
	src
	../third_party/tinyxml
	../third_party/physfs
	../third_party/utfcpp/source
        ${CMAKE_BINARY_DIR}
)
include(sdl2-config OPTIONAL)

# Source Lists
SET(VVV_SRC
	src/BinaryBlob.cpp
	src/BlockV.cpp
	src/editor.cpp
	src/Ent.cpp
	src/Entity.cpp
	src/FileSystemUtils.cpp
	src/Finalclass.cpp
	src/Game.cpp
	src/Graphics.cpp
	src/GraphicsResources.cpp
	src/GraphicsUtil.cpp
	src/Input.cpp
	src/KeyPoll.cpp
	src/Labclass.cpp
	src/Logic.cpp
	src/Map.cpp
	src/Music.cpp
	src/Otherlevel.cpp
	src/preloader.cpp
	src/Screen.cpp
	src/Script.cpp
	src/Scripts.cpp
	src/ScriptX.cpp
	src/SoundSystem.cpp
	src/Spacestation2.cpp
	src/TerminalScripts.cpp
	src/Textbox.cpp
	src/titlerender.cpp
	src/Tower.cpp
	src/UtilityClass.cpp
	src/WarpClass.cpp
	src/main.cpp
	src/Network.c
	src/SteamNetwork.c
	src/GOGNetwork.c
        ${CMAKE_BINARY_DIR}/vce.zip.c
	src/Utilities.cpp
        ${GITREV_TMP}
        src/trace.c
)
SET(XML_SRC
	../third_party/tinyxml/tinystr.cpp
	../third_party/tinyxml/tinyxml.cpp
	../third_party/tinyxml/tinyxmlerror.cpp
	../third_party/tinyxml/tinyxmlparser.cpp
)
SET(PFS_SRC
	../third_party/physfs/physfs.c
	../third_party/physfs/physfs_archiver_dir.c
	../third_party/physfs/physfs_archiver_unpacked.c
	../third_party/physfs/physfs_archiver_zip.c
	../third_party/physfs/physfs_byteorder.c
	../third_party/physfs/physfs_unicode.c
	../third_party/physfs/physfs_platform_posix.c
	../third_party/physfs/physfs_platform_unix.c
	../third_party/physfs/physfs_platform_windows.c
	../third_party/physfs/physfs_platform_haiku.cpp
)
IF(APPLE)
	# Are you noticing a pattern with this Apple crap yet?
	SET(PFS_SRC ${PFS_SRC} ../third_party/physfs/physfs_platform_apple.m)
ENDIF()

if(SWITCH)
    link_directories("${PORTLIBS}/lib/" "${LIBNX}/lib/")
endif()

# Executable information
IF(WIN32)
        message(STATUS "Adding resources")
        set(VVV_SRC ${VVV_SRC} "resources.rc")
	ADD_EXECUTABLE(VVVVVV-CE WIN32 ${VVV_SRC})
ELSE()
	ADD_EXECUTABLE(VVVVVV-CE ${VVV_SRC})
ENDIF()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET_SOURCE_FILES_PROPERTIES(src/Scripts.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)
endif()

SET_SOURCE_FILES_PROPERTIES(src/trace.c PROPERTIES COMPILE_FLAGS -fno-instrument-functions)

# Build options
IF(ENABLE_WARNINGS)
	# The weird syntax is due to CMake generator expressions.
	# Saves quite a few lines and boilerplate at the price of readability.
	TARGET_COMPILE_OPTIONS(VVVVVV-CE PRIVATE
		$<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
			-Wall $<$<BOOL:${ENABLE_WERROR}>:-Werror>>
		$<$<CXX_COMPILER_ID:MSVC>:
			/W4 $<$<BOOL:${ENABLE_WERROR}>:/WX>>)
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -U_FORTIFY_SOURCE")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -U_FORTIFY_SOURCE")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

# Library information
ADD_LIBRARY(tinyxml-static STATIC ${XML_SRC})
ADD_LIBRARY(physfs-static STATIC ${PFS_SRC} ${PFSP_SRC})

# FriBidi (Detection pulled from HHVM)
find_package(PkgConfig)

if (SWITCH)
    set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
endif()

pkg_check_modules(FRIBIDI fribidi)

find_path(FRIBIDI_INCLUDE_DIR
          NAMES fribidi/fribidi.h)

target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${FRIBIDI_INCLUDE_DIR}>")

find_library(FRIBIDI_LIBRARY NAMES fribidi)

# libpng
find_package(PNG)

if(NOT PNG_FOUND)
    find_package(ZLIB)

    include(ExternalProject)
    ExternalProject_Add(
        libpng
        URL ${CMAKE_SOURCE_DIR}/../third_party/libpng-1.6.37.tar.xz
        CMAKE_ARGS
            -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
            -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
            -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
            -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
            -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
            -DPNG_SHARED=OFF
            -DBUILD_SHARED_LIBS=FALSE
            -DSKIP_INSTALL_FILES=1
            -DCMAKE_INSTALL_LIBDIR=lib
        PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libpng
    )
    ExternalProject_Get_Property(libpng INSTALL_DIR)

    if(WIN32)
        set(prefix "")
        set(suffix ".lib")
    elseif(APPLE)
        set(prefix "lib")
        set(suffix ".a")
    else()
        set(prefix "lib")
        set(suffix ".a")
    endif()

    add_library(PNG::PNG SHARED IMPORTED)
    set_target_properties(PNG::PNG PROPERTIES IMPORTED_LOCATION ${INSTALL_DIR}/lib/${prefix}png${suffix})
    find_package(ZLIB)
    target_link_libraries(PNG::PNG INTERFACE ZLIB::ZLIB)
    file(MAKE_DIRECTORY "${INSTALL_DIR}/include")
    target_include_directories(PNG::PNG INTERFACE "${INSTALL_DIR}/include")
endif()

# Static Dependencies
TARGET_LINK_LIBRARIES(VVVVVV-CE physfs-static tinyxml-static fribidi PNG::PNG)

# data.zip path
function(convert_to_cstring_literal var value)
    string(REGEX REPLACE "([\\\$\"])" "\\\\\\1" escaped "${value}")
    set("${var}" "\"${escaped}\"" PARENT_SCOPE)
endfunction()

set(DATA_ZIP_PATH "/usr/share/vvvvvv/data.zip" CACHE STRING "Path to data.zip (optional)")
if (DATA_ZIP_PATH)
    convert_to_cstring_literal(DATA_ZIP_STR "${DATA_ZIP_PATH}")
    target_compile_definitions(VVVVVV-CE PRIVATE "DATA_ZIP_PATH=${DATA_ZIP_STR}")
endif()

# SDL2 Dependency (Detection pulled from FAudio)
if (SWITCH)
        message(STATUS "Using SDL2 from pkg-config")
        pkg_check_modules(SDL2 sdl2)
        pkg_check_modules(SDL2_MIXER SDL2_mixer)
        pkg_check_modules(EGL egl)
        target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_STATIC_INCLUDE_DIRS}>" "$<BUILD_INTERFACE:${SDL2_MIXER_STATIC_INCLUDE_DIRS}>")
        target_link_libraries(VVVVVV-CE ${SDL2_STATIC_LIBRARIES} ${SDL2_MIXER_STATIC_LIBRARIES})
elseif (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES AND (MINGW OR MSYS))
        message(STATUS "Building using MinGW SDL libraries")
	target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
	target_link_libraries(VVVVVV-CE SDL2_mixer mingw32 winmm vorbisfile vorbis ${SDL2_LIBRARIES} shlwapi)
elseif (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES)
	message(STATUS "Using pre-defined SDL2 variables SDL2_INCLUDE_DIRS and SDL2_LIBRARIES")
	target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
	target_link_libraries(VVVVVV-CE ${SDL2_LIBRARIES})
else()
	# Only try to autodetect if both SDL2 variables aren't explicitly set
	find_package(SDL2 CONFIG)
	if (TARGET SDL2::SDL2)
		message(STATUS "Using TARGET SDL2::SDL2")
		target_link_libraries(VVVVVV-CE SDL2::SDL2 SDL2_mixer)
	elseif (TARGET SDL2)
		message(STATUS "Using TARGET SDL2")
		target_link_libraries(VVVVVV-CE SDL2 SDL2_mixer)
	else()
		message(STATUS "No TARGET SDL2::SDL2, or SDL2, using variables")
		target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
		target_link_libraries(VVVVVV-CE ${SDL2_LIBRARIES} SDL2_mixer)
	endif()
endif()

# Yes, more Apple Crap
IF(APPLE)
	FIND_LIBRARY(FOUNDATION NAMES Foundation)
	FIND_LIBRARY(IOKIT NAMES IOKit)
	TARGET_LINK_LIBRARIES(VVVVVV-CE objc ${IOKIT} ${FOUNDATION})
ENDIF()
# But hey, also some Haiku crap
IF(HAIKU)
	FIND_LIBRARY(BE_LIBRARY be)
	FIND_LIBRARY(ROOT_LIBRARY root)
	TARGET_LINK_LIBRARIES(VVVVVV-CE ${BE_LIBRARY} ${ROOT_LIBRARY})
ENDIF()
if (SWITCH)
    find_package(Libnx REQUIRED)
    target_link_libraries(VVVVVV-CE switch::libnx)
    target_compile_options(VVVVVV-CE PUBLIC -fexceptions)
endif()

include(GNUInstallDirs)
install(TARGETS VVVVVV-CE
    EXPORT VVVVVV-CE
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
INSTALL(FILES data/VVVVVV-CE.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
INSTALL(FILES data/VVVVVV.png DESTINATION ${CMAKE_INSTALL_DATADIR}/pixmaps RENAME v6ce.png)
INSTALL(FILES ${CMAKE_BINARY_DIR}/data-zip/data.zip DESTINATION ${CMAKE_INSTALL_DATADIR}/vvvvvv COMPONENT data-zip)

set(CPACK_PACKAGE_VENDOR "v6cord")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE.md")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_GENERATE_SHLIBS ON)
set(CPACK_PACKAGE_CONTACT "leo60228 <iakornfeld@gmail.com>")
set(CPACK_PACKAGE_EXECUTABLES "VVVVVV-CE;VVVVVV-CE")
set(CPACK_MONOLITHIC_INSTALL ON)
set(CPACK_GENERATOR "STGZ;DEB")
include(CPack)

set_target_properties(VVVVVV-CE PROPERTIES
    APP_TITLE VVVVVV-CE
    APP_AUTHOR ${CPACK_PACKAGE_VENDOR}
    APP_VERSION ${CPACK_PACKAGE_VERSION}
    ICON ${CMAKE_CURRENT_SOURCE_DIR}/data/VVVVVV.png)

if (SWITCH)
    include(SwitchTools)
    add_nro_target(VVVVVV-CE)
endif()
