{ pkgs ? import <nixpkgs> {} }:

pkgs.mkShell {
  buildInputs = with pkgs; [
    # C++ toolchain
    gcc
    meson
    ninja
    pkg-config
    
    # GTK4 and Layer Shell
    gtk4
    gtk4-layer-shell
    
    # JSON parsing
    nlohmann_json
    # OR alternatively: jsoncpp
    
    # Development tools
    clang-tools  # for clangd LSP
    gdb
    git
    
    # Wayland dependencies
    wayland
    wayland-protocols
   	wlrctl

    # Hyprland (let's be hyprealistic, if you don't have it then wtf)
    # hyprland
  ];

  shellHook = ''
    echo "HyprTouch C++/GTK4 development environment loaded!"
    echo "Compiler: $(gcc --version | head -n1)"
    echo "Meson: $(meson --version | head -n1)"
    echo ""
    echo "Available tools:"
    echo "  - gcc/g++ (C++ compiler)"
    echo "  - meson (build system)"
    echo "  - ninja (build tool)"
    echo "  - clangd (language server)"
    echo "  - GTK4 + gtk4-layer-shell"
    echo "  - nlohmann_json (JSON parsing)"
    echo ""
    echo "To build:"
    echo "  mkdir build && cd build"
    echo "  meson setup .."
    echo "  ninja"
    echo ""
    echo "To run: ./build/hyprtouch"
    echo ""
    
    # Set up pkg-config paths
    export PKG_CONFIG_PATH="${pkgs.gtk4}/lib/pkgconfig:${pkgs.gtk4-layer-shell}/lib/pkgconfig:$PKG_CONFIG_PATH"
    
    # Set up C++ include paths for LSP
    export CPLUS_INCLUDE_PATH="${pkgs.gtk4.dev}/include:${pkgs.gtk4-layer-shell.dev}/include:${pkgs.nlohmann_json}/include:$CPLUS_INCLUDE_PATH"
  '';
}
