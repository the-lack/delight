
{
  description = "C/C++ development environment with GCC and LLVM/Clang";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          # GNU toolchain
          gcc
          gnumake
#          gdb

          # LLVM toolchain
          llvm
          clang
          clang-tools # clangd and clang-tidy
          lld
#          lldb

          # Build tools
 #         cmake
  #        ninja
          pkg-config
        ];

        shellHook = ''
          echo "GCC + LLVM/Clang development environment"
          echo "gcc:    $(gcc --version | head -n1)"
          echo "clang:  $(clang --version | head -n1)"
          echo "clangd: $(clangd --version | head -n1)"
          echo "lli:    $(lli --version | head -n1)"
        '';
      };
    };
}
