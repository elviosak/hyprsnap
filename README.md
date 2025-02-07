# hyprsnap

A plugin to add aerosnap functionality to floating windows.

https://github.com/user-attachments/assets/f5ef22d1-a893-4924-b7cf-a83b169348b7

# Install

## With `hyprpm`
``` bash
hyprpm add https://github.com/elviosak/hyprsnap
hyprpm enable hyprsnap
```

## Manually
``` bash
git clone https://github.com/elviosak/hyprsnap
cd hyprsnap
make -C hyprsnap all
hyprctl plugin load $(pwd)/hyprsnap/hyprsnap.so

# add the last command to "exec-once" with the full path in your config
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
