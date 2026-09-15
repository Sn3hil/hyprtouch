{
  description = "hyprtouch - keyboard-driven mouse control overlay for Hyprland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        packages.default = pkgs.stdenv.mkDerivation {
          pname = "hyprtouch";
          version = "0.1.0";

          src = ./.;

          nativeBuildInputs = [
            pkgs.meson
            pkgs.ninja
            pkgs.pkg-config
            pkgs.wrapGAppsHook4
            pkgs.makeWrapper
          ];

          buildInputs = [
            pkgs.gtk4
            pkgs.gtk4-layer-shell
            pkgs.nlohmann_json
            pkgs.wlrctl
          ];

          # Force Cairo renderer to avoid EGL/Vulkan errors in sandboxed environments
          postFixup = ''
            wrapProgram $out/bin/hyprtouch \
              --set GSK_RENDERER cairo
          '';

          meta = with pkgs.lib; {
            description = "Keyboard-only grid overlay to control the mouse in Hyprland";
            homepage = "https://github.com/Sn3hil/hyprtouch";
            license = licenses.mit;
            maintainers = [ ];
            platforms = platforms.linux;
          };
        };

        devShells.default = pkgs.mkShell {
          nativeBuildInputs = [
            pkgs.meson
            pkgs.ninja
            pkgs.pkg-config
            pkgs.wrapGAppsHook4
          ];
          buildInputs = [
            pkgs.gtk4
            pkgs.gtk4-layer-shell
            pkgs.nlohmann_json
            pkgs.wlrctl
          ];
        };
      }
    );
}
