# hyprtouch

A keyboard-driven mouse control overlay for Hyprland, built with GTK4 and Layer Shell.

<video src="assets/hyprtouch.mp4" controls title="Demo"></video>

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

The recommended way to use hyprtouch is as a daemon for instant response times.

### Daemon Mode (Recommended)

Start the daemon once (e.g., in your Hyprland config or startup script):

```bash
hyprtouch --daemon &
```

Then bind a key to toggle the overlay:

```bash
# In your hyprland.conf
bind = SUPER, T, exec, hyprtouch --toggle-overlay
```

The overlay will appear instantly. When you click or press Escape, it hides automatically but keeps running in the background.

### CLI Commands

| Command | Description |
|---------|-------------|
| `hyprtouch --daemon` | Start the daemon process |
| `hyprtouch --toggle-overlay` | Toggle overlay visibility (auto-starts daemon if needed) |
| `hyprtouch --show` | Explicitly show the overlay |
| `hyprtouch --hide` | Explicitly hide the overlay |
| `hyprtouch --status` | Check if daemon is running |
| `hyprtouch --stop` | Stop the running daemon |
| `hyprtouch --legacy` | Run in old single-process mode (slower) |

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
- **Escape**: Hide overlay (daemon mode) or exit application (legacy mode)
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