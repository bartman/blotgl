{
    inputs = {
        nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    };

    outputs = { nixpkgs, ... }: let
        pkgs = nixpkgs.legacyPackages.x86_64-linux;
    in {
        devShells.x86_64-linux.default = pkgs.mkShell {
            buildInputs = with pkgs; [
                # Build tools
                gcc
                clang
                cmake
                gnumake
                pkg-config

                # Libraries
                libcxx
                libcxx.dev
                libgcc
                libgcc.lib
                gtest
                mesa
                libglvnd
                glib.dev

                # Development tools
                neovim
                gdb
                jq
                kitty.terminfo

                # Additional for DRM/GBM
                libdrm
                libgbm
                glm

                t-rec
                ttygif
                asciinema
                asciinema-agg
            ];
        };
    };
}
