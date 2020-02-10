{deployAndroidPackage, lib, package, os, autoPatchelfHook, pkgs}:

deployAndroidPackage {
  inherit package os;
  buildInputs = [ autoPatchelfHook ]
    ++ [ pkgs.glibc pkgs.stdenv.cc.cc pkgs.zlib pkgs.openssl.out pkgs.openssl pkgs.ncurses5 ];
  patchInstructions = lib.optionalString (os == "linux") ''
    addAutoPatchelfSearchPath $packageBaseDir/lib
    addAutoPatchelfSearchPath ${pkgs.openssl.out}/lib
    autoPatchelf $packageBaseDir/lib || true
    autoPatchelf $packageBaseDir/bin || true
  '';
}
