{ crossSystem ? null }:
let pkgs = import <nixpkgs> {
  inherit crossSystem;
};
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
      CMAKE_MODULE_PATH = if stdenv.targetPlatform.isWindows then "${sdl}/lib/cmake/SDL2/" else null;
    })
  ) {}
