#pragma once

#include <hyprland/src/plugins/PluginAPI.hpp>
#include <hyprland/src/render/Texture.hpp>

inline HANDLE PHANDLE = nullptr;

struct SGlobalState {
    bool isMouseDown = false;
    bool isDragging = false;
    Vector2D initialSize = { -1, -1 };
    CBox wOffsets;
    PHLWINDOWREF window;
};

inline UP<SGlobalState> g_pGlobalState;
