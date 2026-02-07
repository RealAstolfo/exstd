{
  description = "exstd — Extended C++20 Standard Library";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    bitsery = { url = "github:fraillt/bitsery"; flake = false; };
  };

  outputs = { self, nixpkgs, flake-utils, bitsery }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "exstd";
          version = "0.1.0";
          src = ./.;

          nativeBuildInputs = with pkgs; [ zig gnumake pkg-config ];
          buildInputs = with pkgs; [ zlib ];

          postUnpack = ''
            rm -rf $sourceRoot/vendors
            mkdir -p $sourceRoot/vendors
            cp -r ${bitsery} $sourceRoot/vendors/bitsery
            chmod -R u+w $sourceRoot/vendors
          '';

          # exstd is header-only, no build targets needed for library use
          dontBuild = true;

          installPhase = ''
            mkdir -p $out/include
            cp -r include/* $out/include/
          '';

          passthru.src-with-vendors = pkgs.runCommand "exstd-src" {} ''
            cp -r ${self} $out
            chmod -R u+w $out
            rm -rf $out/vendors
            mkdir -p $out/vendors
            cp -r ${bitsery} $out/vendors/bitsery
            chmod -R u+w $out/vendors
          '';
        };

        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            zig
            gcc
            gnumake
            pkg-config
            zlib
          ];

          shellHook = ''
            if [ ! -d vendors/bitsery ] || [ -L vendors/bitsery ]; then
              rm -rf vendors/bitsery
              mkdir -p vendors
              cp -r ${bitsery} vendors/bitsery
              chmod -R u+w vendors/bitsery
            fi
            echo "exstd development environment"
          '';
        };
      }
    );
}
