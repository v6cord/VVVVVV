{ cross ? false, clang ? false }:
let pkgsNative = import (builtins.fetchTarball {
  name = "cross-compile-nixpkgs";
  url = https://github.com/nixos/nixpkgs/archive/2436c27541b2f52deea3a4c1691216a02152e729.tar.gz;
  sha256 = "0p98dwy3rbvdp6np596sfqnwlra11pif3rbdh02pwdyjmdvkmbvd";
}) {};
    pkgs = if cross then pkgsNative.pkgsCross.mingwW64 else pkgsNative;
    stdenv = if clang then pkgs.llvmPackages_latest.stdenv else pkgs.stdenv;
in
  pkgs.callPackage (
    {smpeg2, mkShell, cmake, pkgsStatic, SDL2, SDL2_mixer, automake, fribidi, pkgconfig, ninja}:
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
        pkgconfig # find fribidi
        ninja # this isn't needed on CI, but it's annoying to disable
        pkgsNative.gdb # we don't want a cross gdb
        pkgsNative.wineWowPackages.unstable # this is my system wine, which makes things a lot easier
      ];
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
      ] else [ SDL2 SDL2_mixer fribidi ];
      CMAKE_MODULE_PATH = if stdenv.targetPlatform.isWindows then "${sdl}/lib/cmake/SDL2/" else "${SDL2.dev}/lib/cmake/SDL2/";
    })
  ) {}
