{ cross ? false }:
let pkgsNative = import (builtins.fetchTarball {
  name = "cross-compile-nixpkgs";
  url = https://github.com/nixos/nixpkgs/archive/2436c27541b2f52deea3a4c1691216a02152e729.tar.gz;
  sha256 = "0p98dwy3rbvdp6np596sfqnwlra11pif3rbdh02pwdyjmdvkmbvd";
}) {};
    pkgs = if cross then pkgsNative.pkgsCross.mingwW64 else pkgsNative;
in
  pkgs.callPackage (
    {stdenv, smpeg2, mkShell, cmake, pkgsStatic, SDL2, SDL2_mixer, automake}:
    mkShell (let sdl = (SDL2.override {x11Support = stdenv.isLinux;}).overrideAttrs (oldAttrs: {
        outputs = ["out"];
        outputBin = "out";
        passthru = {
          dev = sdl;
        };
        postInstall = "rm $out/lib/*.la";
      }); in {
      nativeBuildInputs = [ cmake ]; # you build dependencies here
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
      ] else [ SDL2 SDL2_mixer ];
      CMAKE_MODULE_PATH = if stdenv.targetPlatform.isWindows then "${sdl}/lib/cmake/SDL2/" else "${SDL2.dev}/lib/cmake/SDL2/";
    })
  ) {}
