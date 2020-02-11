cmake_minimum_required(VERSION 3.6)

project(GAME)

# armeabi-v7a requires cpufeatures library
# include(AndroidNdkModules)
# android_ndk_import_module_cpufeatures()


# SDL sources are in a subfolder named "SDL"
#add_subdirectory(SDL)

# Compilation of companion libraries
#add_subdirectory(SDL_image)
#add_subdirectory(SDL_mixer)
#add_subdirectory(SDL_ttf)

include(ExternalProject)

externalproject_add(SDL_build
    URL https://github.com/leo60228/SDL-mirror/archive/c1589de137eb4d64a5defdfbf23fcd11b52d01a3.tar.gz
    URL_HASH SHA256=c174c5ab843b99d825b5b6683d6aaf07808fcb551782a3cc97fdac353cbab7a2
    CMAKE_ARGS
        -DANDROID_ABI=${ANDROID_ABI}
        -DANDROID_PLATFORM=${ANDROID_PLATFORM}
        -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    BUILD_BYPRODUCTS <INSTALL_DIR>/include/SDL2/SDL.h <INSTALL_DIR>/lib/libSDL2.a <INSTALL_DIR>/lib/libSDL2main.a <INSTALL_DIR>/lib/libhidapi.so
)

ExternalProject_Get_Property(SDL_build INSTALL_DIR)
set(SDL2_INCLUDE_DIRS ${INSTALL_DIR}/include ${INSTALL_DIR}/include/SDL2)
set(SDL2_PREFIX ${INSTALL_DIR})

find_library(log-lib log)
find_library(android-lib android)
find_library(GLESv1_CM-lib GLESv1_CM)
find_library(GLESv2-lib GLESv2)

set(HIDAPI ${INSTALL_DIR}/lib/libhidapi.so)
set(SDL2_LIBRARIES ${INSTALL_DIR}/lib/libSDL2.a ${INSTALL_DIR}/lib/libSDL2main.a ${log-lib} ${android-lib} ${GLESv1_CM-lib} ${GLESv2-lib} ${HIDAPI})

add_custom_target(copy-hidapi ALL
    DEPENDS SDL_build
    COMMAND ${CMAKE_COMMAND} -E copy ${HIDAPI} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/
)

externalproject_add(libogg_build
    URL https://ftp.osuosl.org/pub/xiph/releases/ogg/libogg-1.3.4.tar.xz
    URL_HASH SHA256=c163bc12bc300c401b6aa35907ac682671ea376f13ae0969a220f7ddf71893fe
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${ANDROID_TOOLCHAIN_ROOT}
        AR=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ar
        AS=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-as
        CC=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang
        CXX=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang++
        LD=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ld
        RANLIB=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ranlib
        STRIP=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-strip
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --host=aarch64-android-linux --disable-shared --enable-static
    BUILD_BYPRODUCTS <INSTALL_DIR>/include/ogg/ogg.h <INSTALL_DIR>/lib/libogg.a
)

ExternalProject_Get_Property(libogg_build INSTALL_DIR)
set(LIBOGG_LIB_DIR ${INSTALL_DIR}/lib)
set(LIBOGG_INCLUDE_DIR ${INSTALL_DIR}/include)

externalproject_add(libvorbis_build
    URL https://ftp.osuosl.org/pub/xiph/releases/vorbis/libvorbis-1.3.6.tar.xz
    URL_HASH SHA256=af00bb5a784e7c9e69f56823de4637c350643deedaf333d0fa86ecdba6fcb415
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${ANDROID_TOOLCHAIN_ROOT}
        AR=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ar
        AS=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-as
        CC=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang
        CXX=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang++
        LD=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ld
        RANLIB=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ranlib
        STRIP=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-strip
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --host=aarch64-android-linux --disable-shared --enable-static "LDFLAGS=-L${LIBOGG_LIB_DIR}" "CFLAGS=-I${LIBOGG_INCLUDE_DIR}"
        BUILD_BYPRODUCTS <INSTALL_DIR>/include/vorbis/vorbis.h <INSTALL_DIR>/lib/libvorbis.a <INSTALL_DIR>/lib/libvorbisfile.a
)

ExternalProject_Get_Property(libvorbis_build INSTALL_DIR)
set(LIBVORBIS_LIB_DIR ${INSTALL_DIR}/lib)
set(LIBVORBIS_INCLUDE_DIR ${INSTALL_DIR}/include)
add_dependencies(libvorbis_build libogg_build)

externalproject_add(SDL_mixer_build
    URL https://github.com/leo60228/SDL_mixer/archive/b4a3541b8d6f015fb0103cbf94f50a8153974cc7.tar.gz
    URL_HASH SHA256=7c439032d7a24f12f54c0a1fe26f2fe0fea0bc4278e837930496d4387aa00446
    CONFIGURE_COMMAND
        ${CMAKE_COMMAND} -E env
        TOOLCHAIN=${ANDROID_TOOLCHAIN_ROOT}
        AR=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ar
        AS=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-as
        CC=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang
        CXX=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android29-clang++
        LD=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ld
        RANLIB=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-ranlib
        STRIP=${ANDROID_TOOLCHAIN_ROOT}/bin/aarch64-linux-android-strip
        SDL2_CONFIG=${SDL2_PREFIX}/bin/sdl2-config
        <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --host=aarch64-android-linux --disable-shared --enable-static --enable-music-ogg --disable-music-ogg-shared "LDFLAGS=-L${LIBOGG_LIB_DIR} -L${LIBVORBIS_LIB_DIR}" "CFLAGS=-I${LIBOGG_INCLUDE_DIR} -I${LIBVORBIS_INCLUDE_DIR}"
    BUILD_BYPRODUCTS <INSTALL_DIR>/include/SDL_mixer.h <INSTALL_DIR>/lib/libSDL2_mixer.a
)

add_dependencies(SDL_mixer_build SDL_build libogg_build libvorbis_build)

ExternalProject_Get_Property(SDL_mixer_build INSTALL_DIR)
set(SDL2_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS} ${INSTALL_DIR}/include ${INSTALL_DIR}/include/SDL2)
set(SDL2_LIBRARIES ${SDL2_LIBRARIES} ${INSTALL_DIR}/lib/libSDL2_mixer.a ${LIBVORBIS_LIB_DIR}/libvorbisfile.a ${LIBVORBIS_LIB_DIR}/libvorbis.a ${LIBOGG_LIB_DIR}/libogg.a)

#EXTERNALPROJECT_ADD(
#    fribidi_build
#    URL https://github.com/fribidi/fribidi/releases/download/v1.0.8/fribidi-1.0.8.tar.bz2
#    URL_HASH SHA256=94c7b68d86ad2a9613b4dcffe7bbeb03523d63b5b37918bdf2e4ef34195c1e6c
#    CONFIGURE_COMMAND <SOURCE_DIR>/configure --prefix=<INSTALL_DIR> --host=aarch64-android-linux --disable-shared --enable-static && ${CMAKE_COMMAND} -E copy <BINARY_DIR>/config.h <BINARY_DIR>/fribidi-config.h
#    BUILD_BYPRODUCTS <BINARY_DIR>/lib/.libs/libfribidi.a <SOURCE_DIR>/lib/fribidi.h
#)
#
#ExternalProject_Get_Property(fribidi_build BINARY_DIR)
#ExternalProject_Get_Property(fribidi_build SOURCE_DIR)
#set(FRIBIDI_LIBRARY "${BINARY_DIR}/lib/.libs/libfribidi.a")
#set(FRIBIDI_INCLUDE_DIR "${SOURCE_DIR}/lib/" "${BINARY_DIR}")

# Your game and its CMakeLists.txt are in a subfolder named "src"
add_subdirectory(root/desktop_version)

