{ cross ? false, clang ? false, debug ? false, android ? false }:
let pkgsNative = import (builtins.fetchTarball {
  name = "cross-compile-nixpkgs";
  url = https://github.com/nixos/nixpkgs/archive/2436c27541b2f52deea3a4c1691216a02152e729.tar.gz;
  sha256 = "0p98dwy3rbvdp6np596sfqnwlra11pif3rbdh02pwdyjmdvkmbvd";
}) { config.android_sdk.accept_license = true; };
    pkgs = if cross then pkgsNative.pkgsCross.mingwW64 else pkgsNative;
    stdenv = if clang then pkgs.llvmPackages_latest.stdenv else pkgs.stdenv;
    androidenv = pkgsNative.callPackage ../third_party/androidenv {};
    composeAndroidPackages = androidenv.composeAndroidPackages;
    androidPkgs = composeAndroidPackages {
        buildToolsVersions = [ "29.0.3" ];
        cmakeVersions = [ "3.10.2" ];
        lldbVersions = [ "3.1.4508709" ];
        ndkVersion = "21.0.6113669";
        includeNDK = true;
        platformVersions = [ "29" ];
        platformToolsVersion = "29.0.5";
        toolsVersion = "26.1.1";
        includeSystemImages = false;
        includeSources = false;
        useGoogleAPIs = false;
        useGoogleTVAddOns = false;
    };
in
  pkgs.callPackage (
    {smpeg2, mkShell, cmake, pkgsStatic, SDL2, SDL2_mixer, automake, fribidi, pkgconfig, ninja, zlib, libpng, libicns, imagemagick}:
    (mkShell.override { inherit stdenv; }) (let sdl = (SDL2.override {x11Support = stdenv.isLinux;}).overrideAttrs (oldAttrs: {
        outputs = ["out"];
        outputBin = "out";
        passthru = {
          dev = sdl;
        };
        postInstall = "rm $out/lib/*.la";
      }); in {
      nativeBuildInputs = [
        cmake # build generator
        ninja
        pkgsNative.gdb # we don't want a cross gdb
      ] ++ (if debug then [
        pkgsNative.wineWowPackages.unstable # this is my system wine, which makes things a lot easier
        pkgsNative.libicns pkgsNative.imagemagick # icon conversion
        pkgsNative.gitAndTools.git-subrepo # subrepo management
      ] else []) ++ (if android then [
        androidPkgs.platform-tools
        androidPkgs.ndk-bundle
        androidPkgs.platforms
        androidPkgs.androidsdk
        pkgsNative.openjdk8
        pkgsNative.ant
      ] else [
        pkgconfig # find fribidi
      ]);
      buildInputs = if stdenv.targetPlatform.isWindows then [
        sdl
        ((SDL2_mixer.override {
          SDL2 = sdl;
          smpeg2 = (smpeg2.override { SDL2 = sdl; }).overrideAttrs (oldAttrs: {
            meta = oldAttrs.meta // {
              platforms = stdenv.lib.platforms.all;
            };
          });
          libmodplug = null;
          fluidsynth = null;
          flac = null;
        }).overrideAttrs (oldAttrs: {
          meta = oldAttrs.meta // {
            platforms = stdenv.lib.platforms.all;
          };
          autoreconfFlags = "-I${sdl}/share/aclocal/ --install --force --verbose";
          configureFlags = oldAttrs.configureFlags ++ [ "--disable-shared" ];
          patches = [ ./windres.patch ];
        }))
        (pkgsStatic.fribidi.overrideAttrs (oldAttrs: {
          meta = oldAttrs.meta // {
            platforms = stdenv.lib.platforms.all;
          };
        }))
        zlib
        libpng
      ] else if android then [] else [ SDL2 SDL2_mixer fribidi zlib libpng ];
      CMAKE_MODULE_PATH = if stdenv.targetPlatform.isWindows then "${sdl}/lib/cmake/SDL2/" else "${SDL2.dev}/lib/cmake/SDL2/";
    } // (if android then rec {
      ANDROID_NDK_HOME = "${androidPkgs.ndk-bundle}/libexec/android-sdk/ndk-bundle";
      NDK = ANDROID_NDK_HOME;
      ANDROID_HOME = "${androidPkgs.androidsdk}/libexec/android-sdk";
      CPATH = "${ANDROID_NDK_HOME}/sysroot/usr/include/aarch64-linux-android";
      HOST_TAG = "linux-x86_64";
      TARGET = "aarch64-linux-android";
      TOOLCHAIN = "${NDK}/toolchains/llvm/prebuilt/${HOST_TAG}";
      AR = "${TOOLCHAIN}/bin/aarch64-linux-android-ar";
      AS = "${TOOLCHAIN}/bin/aarch64-linux-android-as";
      CC = "${TOOLCHAIN}/bin/aarch64-linux-android21-clang";
      CXX = "${TOOLCHAIN}/bin/aarch64-linux-android21-clang++";
      LD = "${TOOLCHAIN}/bin/aarch64-linux-android-ld";
      RANLIB = "${TOOLCHAIN}/bin/aarch64-linux-android-ranlib";
      STRIP = "${TOOLCHAIN}/bin/aarch64-linux-android-strip";
      shellHook = ''
        export LD_LIBRARY_PATH=${pkgsNative.zlib}/lib:$LD_LIBRARY_PATH
        export AR="${AR}"
        export AS="${AS}"
        export CC="${CC}"
        export CXX="${CXX}"
        export LD="${LD}"
        export RANLIB="${RANLIB}"
        export STRIP="${STRIP}"
      '';
    } else {}))
  ) {}
