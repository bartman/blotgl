{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { nixpkgs, ... }: let
    pkgs = nixpkgs.legacyPackages.x86_64-linux;
  in {
    devShells.x86_64-linux.default = pkgs.mkShell {
      buildInputs = [
        # Build tools
        pkgs.clang
        pkgs.cmake
        pkgs.gnumake
        pkgs.pkg-config

        # Libraries
        pkgs.gtest
        pkgs.mesa
        pkgs.libglvnd

        # Development tools
        pkgs.neovim
        pkgs.gdb
        pkgs.jq
        pkgs.kitty.terminfo

        # Additional for DRM/GBM
        pkgs.libdrm
        pkgs.libgbm
      ];
    };
  };
}
