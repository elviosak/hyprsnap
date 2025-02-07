# hyprsnap

A plugin to add aerosnap-like functionality to floating windows.

https://github.com/user-attachments/assets/53f19b18-0064-4b39-941d-6683a6cfa386

# Install
```
hyprpm add https://github.com/elviosak/hyprsnap
hyprpm enable hyprsnap
```

# Config
``` toml
plugins {
  hyprsnap {
    enabled = 1             # 0 or 1, enables/disables the plugin
    top_maximize = 1        # 0 or 1, enables/disables maximizing window when snapping to top
    h_distance = 30         # horizontal distance from the screen edge to snap the window
    v_distance = 20         # vertical distance from the screen edge to snap the window
    h_corner_distance = 400 # horizontal distance from the screen edge to snap the window when already snapped vertically
    v_corner_distance = 300 # vertical distance from the screen edge to snap the window  when already snapped horizontally
  }
}
```
# License
License: [LGPL-2.1+](LICENSE "License")
