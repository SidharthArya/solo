{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    nixpkgs-wayland = {
      url = "github:nix-community/nixpkgs-wayland";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs = { systems, nixpkgs, nixpkgs-wayland, ... }@inputs:
    let
      pkgs = nixpkgs.legacyPackages.x86_64-linux;
      pkgsw = nixpkgs-wayland.pkgs;
      commonBuildInputs = [
        pkgs.ninja
        pkgs.clang
        pkgs.meson
        pkgs.wayland-scanner
        pkgs.wayland
        pkgs.pixman
        pkgs.libxkbcommon
        pkgs.pkg-config
        pkgs.wayland-protocols
        # pkgs.wlroots
        inputs.nixpkgs-wayland.packages.x86_64-linux.wlroots
      ];
    in {
      nixpkgs.overlays = [ inputs.nixpkgs-wayland.overlay ];
      nix.settings = {
        # add binary caches
        trusted-public-keys = [
          "cache.nixos.org-1:6NCHdD59X431o0gWypbMrAURkbJ16ZPMQFGspcDShjY="
          "nixpkgs-wayland.cachix.org-1:3lwxaILxMRkVhehr5StQprHdEo4IrE8sRho9R9HOLYA="
        ];
        substituters =
          [ "https://cache.nixos.org" "https://nixpkgs-wayland.cachix.org" ];
      };

      devShells.x86_64-linux.default =
        pkgs.mkShell { nativeBuildInputs = commonBuildInputs; };
      packages.x86_64-linux.default = pkgs.stdenv.mkDerivation {
        name = "solo";
        src = ./.;
        buildInputs = commonBuildInputs;
        buildPhase = ''
          ninja
        '';
        installPhase = ''
          mkdir -p $out/bin;
          cp solo $out/bin;
        '';

      };
    };
}
