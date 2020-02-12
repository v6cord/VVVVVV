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
OPTION(ENABLE_FRIBIDI "Enable Fribidi" ON)

SET(CUSTOM_LEVEL_SUPPORT ENABLED CACHE STRING "Optionally disable playing and/or editing of custom levels")
SET_PROPERTY(CACHE CUSTOM_LEVEL_SUPPORT PROPERTY STRINGS ENABLED NO_EDITOR DISABLED)

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
        ../third_party/cparse
        ${CMAKE_BINARY_DIR}
)
include(sdl2-config OPTIONAL)

# Source Lists
SET(VVV_SRC
	src/BinaryBlob.cpp
	src/BlockV.cpp
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
IF(NOT CUSTOM_LEVEL_SUPPORT STREQUAL "DISABLED")
	LIST(APPEND VVV_SRC src/editor.cpp)
ENDIF()

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
SET(PARSE_SRC
        ../third_party/cparse/shunting-yard.cpp
        ../third_party/cparse/packToken.cpp
        ../third_party/cparse/functions.cpp
        ../third_party/cparse/containers.cpp
)
IF(APPLE)
	# Are you noticing a pattern with this Apple crap yet?
	SET(PFS_SRC ${PFS_SRC} ../third_party/physfs/physfs_platform_apple.m)
ENDIF()

if(SWITCH)
    link_directories("${PORTLIBS}/lib/" "${LIBNX}/lib/")
endif()
if(ANDROID)
    link_directories("${CMAKE_INSTALL_PREFIX}/lib/")
endif()

# Executable information
IF(WIN32)
        message(STATUS "Adding resources")
        set(VVV_SRC ${VVV_SRC} "resources.rc")
	ADD_EXECUTABLE(VVVVVV-CE WIN32 ${VVV_SRC})
ELSEIF(ANDROID)
        ADD_LIBRARY(main SHARED ${VVV_SRC})
ELSE()
	ADD_EXECUTABLE(VVVVVV-CE ${VVV_SRC})
ENDIF()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    SET_SOURCE_FILES_PROPERTIES(src/Scripts.cpp PROPERTIES COMPILE_FLAGS -fno-var-tracking-assignments)
endif()

if(ENABLE_PROFILE)
    SET_SOURCE_FILES_PROPERTIES(src/trace.c PROPERTIES COMPILE_FLAGS -fno-instrument-functions)
endif()

# Build options
IF(ENABLE_WARNINGS)
        if(NOT ANDROID)
            # The weird syntax is due to CMake generator expressions.
            # Saves quite a few lines and boilerplate at the price of readability.
            TARGET_COMPILE_OPTIONS(VVVVVV-CE PRIVATE
                    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
                            -Wall $<$<BOOL:${ENABLE_WERROR}>:-Werror>>
                    $<$<CXX_COMPILER_ID:MSVC>:
                            /W4 $<$<BOOL:${ENABLE_WERROR}>:/WX>>)
        else()
            TARGET_COMPILE_OPTIONS(main PRIVATE
                    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
                            -Wall $<$<BOOL:${ENABLE_WERROR}>:-Werror>>
                    $<$<CXX_COMPILER_ID:MSVC>:
                            /W4 $<$<BOOL:${ENABLE_WERROR}>:/WX>>)
        endif()
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -U_FORTIFY_SOURCE")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -U_FORTIFY_SOURCE")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

IF(CUSTOM_LEVEL_SUPPORT STREQUAL "NO_EDITOR")
	ADD_DEFINITIONS(-DNO_EDITOR)
ELSEIF(CUSTOM_LEVEL_SUPPORT STREQUAL "DISABLED")
	ADD_DEFINITIONS(-DNO_CUSTOM_LEVELS -DNO_EDITOR)
ENDIF()

# Library information
ADD_LIBRARY(tinyxml-static STATIC ${XML_SRC})
ADD_LIBRARY(physfs-static STATIC ${PFS_SRC} ${PFSP_SRC})
ADD_LIBRARY(cparse-static STATIC ${PARSE_SRC})

if (3DS)
    find_package(CTRULIB REQUIRED)
    target_link_libraries(VVVVVV-CE 3ds::ctrulib)
    target_link_libraries(physfs-static 3ds::ctrulib)
endif()

if (SWITCH)
    target_compile_options(cparse-static PUBLIC -fexceptions)
endif()

if(ANDROID)
    set_target_properties(tinyxml-static PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(physfs-static PROPERTIES LINKER_LANGUAGE CXX)
    set_target_properties(cparse-static PROPERTIES LINKER_LANGUAGE CXX)
endif()

# FriBidi (Detection pulled from HHVM)
find_package(PkgConfig)

if (SWITCH)
    set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
endif()
if (ENABLE_FRIBIDI AND NOT ANDROID AND NOT 3DS)
    pkg_check_modules(FRIBIDI fribidi)

    find_path(FRIBIDI_INCLUDE_DIR
            NAMES fribidi/fribidi.h)

    find_library(FRIBIDI_LIBRARY NAMES fribidi)

    target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${FRIBIDI_INCLUDE_DIR}>")

    set(LINK_FRIBIDI fribidi)
else()
    add_definitions(-DNO_FRIBIDI=1)
    set(LINK_FRIBIDI "")
endif()

if(ANDROID OR 3DS OR SWITCH)
    add_definitions(-DSTRICT_SDL_PATH=1)
endif()
if(3DS)
    add_definitions(-DNO_SDL_MIXER)
endif()

# libpng
find_package(PNG)

if(NOT PNG_FOUND)
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

    find_package(ZLIB)

    include(ExternalProject)
    if (ANDROID)
        ExternalProject_Add(
            libpng
            URL ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libpng-1.6.37.tar.xz
            CMAKE_ARGS
                -DANDROID_ABI=${ANDROID_ABI}
                -DANDROID_PLATFORM=${ANDROID_PLATFORM}
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
            BUILD_BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/libpng/lib/${prefix}png${suffix}
                ${CMAKE_CURRENT_BINARY_DIR}/libpng/include/png.h
        )
    else()
        ExternalProject_Add(
            libpng
            URL ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/libpng-1.6.37.tar.xz
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
            BUILD_BYPRODUCTS
                ${CMAKE_CURRENT_BINARY_DIR}/libpng/lib/${prefix}png${suffix}
                ${CMAKE_CURRENT_BINARY_DIR}/libpng/include/png.h
        )
    endif()

    ExternalProject_Get_Property(libpng INSTALL_DIR)

    add_library(PNG::PNG SHARED IMPORTED)
    set_target_properties(PNG::PNG PROPERTIES IMPORTED_LOCATION ${INSTALL_DIR}/lib/${prefix}png${suffix})
    find_package(ZLIB)
    set_target_properties(PNG::PNG PROPERTIES INTERFACE_LINK_LIBRARIES ZLIB::ZLIB)
    file(MAKE_DIRECTORY "${INSTALL_DIR}/include")
    set_target_properties(PNG::PNG PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${INSTALL_DIR}/include")
    add_dependencies(PNG::PNG libpng)
endif()

if (3DS)
    include_directories(../third_party/ctr_pthread/)
else()
    find_package(Threads)
endif()

# Static Dependencies
if(ANDROID)
    TARGET_LINK_LIBRARIES(main physfs-static tinyxml-static cparse-static PNG::PNG Threads::Threads)
    add_dependencies(main SDL_build SDL_mixer_build)
else()
    TARGET_LINK_LIBRARIES(VVVVVV-CE physfs-static tinyxml-static cparse-static "${LINK_FRIBIDI}" PNG::PNG Threads::Threads)
endif()

# data.zip path
function(convert_to_cstring_literal var value)
    string(REGEX REPLACE "([\\\$\"])" "\\\\\\1" escaped "${value}")
    set("${var}" "\"${escaped}\"" PARENT_SCOPE)
endfunction()

set(DATA_ZIP_PATH "/usr/share/vvvvvv/data.zip" CACHE STRING "Path to data.zip (optional)")
if (DATA_ZIP_PATH)
    convert_to_cstring_literal(DATA_ZIP_STR "${DATA_ZIP_PATH}")
    if (ANDROID)
        target_compile_definitions(main PRIVATE "DATA_ZIP_PATH=${DATA_ZIP_STR}")
    else()
        target_compile_definitions(VVVVVV-CE PRIVATE "DATA_ZIP_PATH=${DATA_ZIP_STR}")
    endif()
endif()

# SDL2 Dependency (Detection pulled from FAudio)
if (SWITCH)
        message(STATUS "Using SDL2 from pkg-config")
        pkg_check_modules(SDL2 sdl2 REQUIRED)
        pkg_check_modules(SDL2_MIXER SDL2_mixer REQUIRED)
        pkg_check_modules(EGL egl REQUIRED)
        target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_STATIC_INCLUDE_DIRS}>" "$<BUILD_INTERFACE:${SDL2_MIXER_STATIC_INCLUDE_DIRS}>")
        target_link_libraries(VVVVVV-CE ${SDL2_STATIC_LIBRARIES} ${SDL2_MIXER_STATIC_LIBRARIES})
elseif (3DS)
    message(STATUS "Locating SDL-3DS")
    find_library(SDL2_LIBRARIES NAMES libSDL2.a)
    find_path(SDL2_INCLUDE_DIRS NAMES SDL2/SDL.h)
    target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
    target_link_libraries(VVVVVV-CE ${SDL2_LIBRARIES})
elseif (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES AND (MINGW OR MSYS))
        message(STATUS "Building using MinGW SDL libraries")

        # mingw-std-threads
        add_subdirectory(../third_party/mingw-std-threads ${CMAKE_CURRENT_BINARY_DIR}/mingw-std-threads)

	target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
	target_link_libraries(VVVVVV-CE SDL2_mixer mingw32 winmm vorbisfile vorbis ${SDL2_LIBRARIES} shlwapi mingw_stdthreads)
elseif (DEFINED SDL2_INCLUDE_DIRS AND DEFINED SDL2_LIBRARIES)
	message(STATUS "Using pre-defined SDL2 variables SDL2_INCLUDE_DIRS and SDL2_LIBRARIES")
        if(ANDROID)
            target_include_directories(main PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
            target_link_libraries(main ${SDL2_LIBRARIES})
        else()
            target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
            target_link_libraries(VVVVVV-CE ${SDL2_LIBRARIES})
        endif()
else()
	# Only try to autodetect if both SDL2 variables aren't explicitly set
	find_package(SDL2 CONFIG)
	if (TARGET SDL2::SDL2)
		message(STATUS "Using TARGET SDL2::SDL2")
                if(ANDROID)
                    target_link_libraries(main SDL2::SDL2 SDL2_mixer vorbisfile vorbis ogg)
                else()
                    target_link_libraries(VVVVVV-CE SDL2::SDL2 SDL2_mixer)
                endif()
	elseif (TARGET SDL2)
		message(STATUS "Using TARGET SDL2")
                if(ANDROID)
                    target_link_libraries(main SDL2 SDL2_mixer vorbisfile vorbis ogg)
                else()
                    target_link_libraries(VVVVVV-CE SDL2 SDL2_mixer)
                endif()
	else()
		message(STATUS "No TARGET SDL2::SDL2, or SDL2, using variables")
                if(ANDROID)
                    target_include_directories(main PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
                    target_link_libraries(main ${SDL2_LIBRARIES} SDL2_mixer vorbisfile vorbis ogg)
                else()
                    target_include_directories(VVVVVV-CE PUBLIC "$<BUILD_INTERFACE:${SDL2_INCLUDE_DIRS}>")
                    target_link_libraries(VVVVVV-CE ${SDL2_LIBRARIES} SDL2_mixer)
                endif()
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
endif()
if (SWITCH OR 3DS)
    target_compile_options(VVVVVV-CE PUBLIC -fexceptions)
endif()
if(ANDROID)
    find_library(log-lib log)
    target_link_libraries(main ${log-lib})
    add_dependencies(main copy-hidapi)
endif()

if (NOT ANDROID)
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
else()
    include(GNUInstallDirs)
    install(TARGETS main
        EXPORT main
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()
