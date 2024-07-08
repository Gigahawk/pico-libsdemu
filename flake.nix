{
  description = "Basic rpi pico development shell";
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/master";
  inputs.flake-utils.url = "github:numtide/flake-utils";

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
    let
      pico-sdk151 = with pkgs; (pico-sdk.overrideAttrs (o:
        rec {
        pname = "pico-sdk";
        version = "1.5.1";
        src = fetchFromGitHub {
          fetchSubmodules = true;
          # Hack to support Adafruit Feather RP2040 with USB A Host
          owner = "Gigahawk";
          repo = pname;
          rev = "79648dbc708dccd33abe3a1fe9e495d12c508445";
          sha256 = "sha256-gLC0FX4aJT0+srjoObMx68Pc4RzqmakJqrPqFLzyM8Q=";
        };
        }));
      pkgs = nixpkgs.legacyPackages.${system};
    in {
        devShell = pkgs.mkShell {
          nativeBuildInputs = with pkgs; [
            pico-sdk151
            python3
            cmake
            clang-tools
            gcc-arm-embedded
            ];
          shellHook = ''
            export PICO_SDK_PATH="${pico-sdk151}/lib/pico-sdk"
            '';
          };
      });
}