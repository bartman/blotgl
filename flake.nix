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
        pkgs.gcc
        pkgs.clang
        pkgs.cmake
        pkgs.gnumake
        pkgs.pkg-config

        # Libraries
        pkgs.libcxx
        pkgs.libcxx.dev
        pkgs.libgcc
        pkgs.libgcc.lib
        pkgs.gtest
        pkgs.mesa
        pkgs.libglvnd
        pkgs.glib.dev

        # Development tools
        pkgs.neovim
        pkgs.gdb
        pkgs.jq
        pkgs.kitty.terminfo

        # Additional for DRM/GBM
        pkgs.libdrm
        pkgs.libgbm

        pkgs.t-rec
        pkgs.ttygif
        pkgs.asciinema
        pkgs.asciinema-agg
      ];
    };
  };
}
