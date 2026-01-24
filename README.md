# hyprtouch

A keyboard-driven mouse control overlay for Hyprland, built with GTK4 and Layer Shell.


## Installation & Build

### Using Nix (Recommended)

This project includes a `flake.nix` for easy development and building.

```bash
# Build the package
nix build

# Run directly
nix run .

# Install the package globally
nix profile add .
```

### Manual Build

Requirements:
- GTK4
- gtk4-layer-shell
- nlohmann_json
- wlrctl
- meson
- ninja
- gcc/clang

```bash
# Setup build directory
meson setup build

# Compile
ninja -C build

# Run
./build/hyprtouch
```

## Usage

```bash
hyprtouch
```

Launch the application to display a grid overlay on your focused monitor. Use keyboard shortcuts to navigate and click at specific locations.

### Input Options

#### Grid Navigation
- **Row selection**: `d`, `f`, `g`, `h`, `j`, `k` (maps to rows 0-5)
- **Column selection**: `a`, `s`, `d`, `f`, `g`, `h`, `j`, `k`, `l`, `;` (maps to columns 0-9)
- **Arrow navigation**: Arrow keys or Shift + Vim/WASD keys
  - Left/Right: Move between columns
  - Up/Down: Move between rows

#### Click Actions
- **Left click**: Space or Enter
- **Right click**: Alt + Space/Enter  
- **Middle click**: Ctrl + Space/Enter
- **Double click**: Shift + Space/Enter

#### Other Controls
- **Escape**: Cancel current selection or exit application
- **Multi-level zoom**: Grid zooms in on selection for precision (2 levels max)

### Grid Labels

Each cell displays a two-character label:
- First character: Row identifier (d, f, g, h, j, k)
- Second character: Column identifier (a, s, d, f, g, h, j, k, l, ;)

Example: Cell "da" is row 0, column 0; cell "k;" is row 5, column 9

### Examples

**Basic usage:**
```bash
hyprtouch
# Press 'd' then 'a' to select top-left cell
# Press Space to left-click at that location
```

**Precision clicking:**
```bash
hyprtouch  
# Select cell to zoom in, then select sub-cell for precise positioning
# Use Shift+Enter for double-click when needed
```