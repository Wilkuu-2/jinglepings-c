with (import <nixpkgs> {});
let
  libs = [
   ];
in 
mkShell {
      packages = [clang-tools clang-manpages clang bear];
      buildInputs = libs;
      LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath libs;
}
