#define WLR_USE_UNSTABLE

#include <unistd.h>

#include <any>
#include <hyprland/src/Compositor.hpp>
#include <hyprland/src/config/ConfigManager.hpp>
#include <hyprland/src/config/ConfigValue.hpp>
#include <hyprland/src/desktop/Window.hpp>
#include <hyprland/src/managers/LayoutManager.hpp>
#include <hyprland/src/managers/input/InputManager.hpp>
#include <hyprland/src/render/Renderer.hpp>
#include <hyprland/src/render/decorations/DecorationPositioner.hpp>

#include "globals.hpp"

// Do NOT change this function.
APICALL EXPORT std::string PLUGIN_API_VERSION() {
    return HYPRLAND_API_VERSION;
}

static void onMouseButton(SCallbackInfo& info, IPointer::SButtonEvent e) {
    // button up
    if (e.state != WL_POINTER_BUTTON_STATE_PRESSED) {

        g_pGlobalState->isMouseDown = false;
        if (g_pGlobalState->isDragging) {
            if (!g_pGlobalState->window.expired()) {
                g_pGlobalState->window->activate(true);
            }
            g_pInputManager->dragMode = MBIND_INVALID;
            g_pLayoutManager->getCurrentLayout()->onEndDragWindow();
            info.cancelled = true;

            g_pGlobalState->isDragging = false;
            g_pGlobalState->window.reset();
        }
        return;
    }
    g_pGlobalState->isMouseDown = true;
}

static void onMouseMove(Vector2D c) {
    static auto* const PTOPMAXIMIZE = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:top_maximize")->getDataStaticPtr();
    static auto* const PENABLED     = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:enabled")->getDataStaticPtr();
    static auto* const PHDISTANCE   = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:h_distance")->getDataStaticPtr();
    static auto* const PVDISTANCE   = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:v_distance")->getDataStaticPtr();
    static auto* const PHCDISTANCE  = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:h_corner_distance")->getDataStaticPtr();
    static auto* const PVCDISTANCE  = (Hyprlang::INT* const*)HyprlandAPI::getConfigValue(PHANDLE, "plugin:hyprsnap:v_corner_distance")->getDataStaticPtr();

    static auto        PGAPSINDATA  = CConfigValue<Hyprlang::CUSTOMTYPE>("general:gaps_in");
    static auto        PGAPSOUTDATA = CConfigValue<Hyprlang::CUSTOMTYPE>("general:gaps_out");
    auto* const        PGAPSIN      = (CCssGapData*)(PGAPSINDATA.ptr())->getData();
    auto* const        PGAPSOUT     = (CCssGapData*)(PGAPSOUTDATA.ptr())->getData();

    const bool         topMaximize = **PTOPMAXIMIZE > 0;
    const bool         enabled     = **PENABLED > 0;
    const int          hDist       = **PHDISTANCE;
    const int          vDist       = **PVDISTANCE;
    const int          hCorner     = **PHCDISTANCE;
    const int          vCorner     = **PVCDISTANCE;

    if (enabled && g_pGlobalState->isMouseDown) {
        auto w = g_pInputManager->currentlyDraggedWindow.lock();

        // currently dragging a window
        if (nullptr != w.get() && w->m_bIsFloating && !w->m_bDraggingTiled && (g_pInputManager->dragMode == MBIND_INVALID || g_pInputManager->dragMode == MBIND_MOVE)) {
            const auto mon = g_pCompositor->m_pLastMonitor ? g_pCompositor->m_pLastMonitor.lock() : g_pCompositor->getMonitorFromCursor();
            if (!mon)
                return;

            g_pGlobalState->isDragging = true;
            const auto screen          = mon->logicalBox();
            const auto resTL           = mon->vecReservedTopLeft;
            const auto resBR           = mon->vecReservedBottomRight;

            // is start of drag, save initial data
            if (g_pGlobalState->window.expired()) {

                g_pGlobalState->window = w;
                Vector2D realSize      = w->m_vRealSize->goal();
                Vector2D realPos       = w->m_vRealPosition->goal();
                CBox     fullBox       = g_pDecorationPositioner->getBoxWithIncludedDecos(w);

                CBox     wOff;
                wOff.x      = fullBox.x - realPos.x;
                wOff.y      = fullBox.y - realPos.y;
                wOff.width  = fullBox.width - realSize.x;
                wOff.height = fullBox.height - realSize.y;
                auto s      = fullBox;

                g_pGlobalState->wOffsets    = wOff;
                auto maxSize                = Vector2D{screen.width - resTL.x - resBR.x - wOff.width, screen.height - resTL.y - resBR.y - wOff.height};
                g_pGlobalState->initialSize = realSize.clamp({1, 1}, maxSize);
            } else { // continue move and check snap regions

                // window initial geometry is all non reserved space on screen.
                CBox wBox;
                wBox.x      = screen.x + resTL.x + PGAPSOUT->left;
                wBox.y      = screen.y + resTL.y + PGAPSOUT->top;
                wBox.width  = screen.width - (resTL.x + resBR.x + PGAPSOUT->left + PGAPSOUT->right);
                wBox.height = screen.height - (resTL.y + resBR.y + PGAPSOUT->top + PGAPSOUT->bottom);

                bool left   = c.x >= screen.x && c.x <= screen.x + hDist + resTL.x;
                bool right  = c.x <= screen.x + screen.width && c.x >= screen.x + screen.width - hDist - resBR.x;
                bool top    = c.y >= screen.y && c.y <= screen.y + vDist + resTL.y;
                bool bottom = c.y <= screen.y + screen.height && c.y >= screen.y + screen.height - vDist - resBR.y;

                // additionally check the corner distances
                if (left && !top && !bottom) {
                    top    = c.y >= screen.y && c.y <= screen.y + vCorner + resTL.y;
                    bottom = c.y <= screen.y + screen.height && c.y >= screen.y + screen.height - vCorner - resBR.y;
                } else if (right && !top && !bottom) {
                    top    = c.y >= screen.y && c.y <= screen.y + vCorner + resTL.y;
                    bottom = c.y <= screen.y + screen.height && c.y >= screen.y + screen.height - vCorner - resBR.y;
                } else if (top && !left && !right) {
                    left  = c.x >= screen.x && c.x <= screen.x + hCorner + resTL.x;
                    right = c.x <= screen.x + screen.width && c.x >= screen.x + screen.width - hCorner - resBR.x;
                } else if (bottom && !left && !right) {
                    left  = c.x >= screen.x && c.x <= screen.x + hCorner + resTL.x;
                    right = c.x <= screen.x + screen.width && c.x >= screen.x + screen.width - hCorner - resBR.x;
                }

                // check any snap
                if (left || right || top || bottom) {
                    if (top && topMaximize && !left && !right) {
                        // wBox is already maximized
                    } else {
                        if (left) {
                            wBox.width = wBox.width / 2 - PGAPSIN->right;
                        } else if (right) {
                            wBox.width = wBox.width / 2 - PGAPSIN->left;
                            wBox.x     = wBox.x + wBox.width + PGAPSIN->left;
                        }
                        if (top) {
                            wBox.height = wBox.height / 2 - PGAPSIN->bottom;
                        } else if (bottom) {
                            wBox.height = wBox.height / 2 - PGAPSIN->top;
                            wBox.y      = wBox.y + wBox.height + PGAPSIN->top;
                        }
                    }
                    wBox.x -= g_pGlobalState->wOffsets.x;
                    wBox.y -= g_pGlobalState->wOffsets.y;
                    wBox.width -= g_pGlobalState->wOffsets.width;
                    wBox.height -= g_pGlobalState->wOffsets.height;

                    g_pGlobalState->window->m_vPosition = wBox.pos();
                    g_pGlobalState->window->m_vSize     = wBox.size();

                    *g_pGlobalState->window->m_vRealPosition = wBox.pos();
                    *g_pGlobalState->window->m_vRealSize     = wBox.size();

                    g_pHyprRenderer->damageWindow(g_pGlobalState->window.lock(), true);
                    g_pHyprRenderer->renderMonitor(mon);
                    g_pGlobalState->window->updateToplevel();

                    g_pInputManager->dragMode = MBIND_INVALID;
                } else {
                    if (g_pInputManager->dragMode == MBIND_INVALID) {
                        // un-snap
                        *g_pGlobalState->window->m_vRealSize = g_pGlobalState->initialSize;
                        g_pHyprRenderer->damageWindow(g_pGlobalState->window.lock());
                        g_pInputManager->dragMode = MBIND_MOVE;
                    }
                }
            }
        }
    }
}

APICALL EXPORT PLUGIN_DESCRIPTION_INFO PLUGIN_INIT(HANDLE handle) {
    PHANDLE = handle;

    const std::string HASH = __hyprland_api_get_hash();

    if (HASH != GIT_COMMIT_HASH) {
        HyprlandAPI::addNotification(PHANDLE, "[hyprsnap] Failure in initialization: Version mismatch (headers ver is not equal to running hyprland ver)",
                                     CHyprColor{1.0, 0.2, 0.2, 1.0}, 5000);
        throw std::runtime_error("[hb] Version mismatch");
    }

    g_pGlobalState = makeUnique<SGlobalState>();

    static auto m_pMouseButtonCallback = HyprlandAPI::registerCallbackDynamic(
        PHANDLE, "mouseButton", [&](void* self, SCallbackInfo& info, std::any param) { onMouseButton(info, std::any_cast<IPointer::SButtonEvent>(param)); });
    static auto m_pMouseMoveCallback = HyprlandAPI::registerCallbackDynamic( //
        PHANDLE, "mouseMove", [&](void* self, SCallbackInfo& info, std::any param) { onMouseMove(std::any_cast<Vector2D>(param)); });

    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:enabled", Hyprlang::INT{1});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:top_maximize", Hyprlang::INT{1});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:h_distance", Hyprlang::INT{30});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:v_distance", Hyprlang::INT{20});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:h_corner_distance", Hyprlang::INT{400});
    HyprlandAPI::addConfigValue(PHANDLE, "plugin:hyprsnap:v_corner_distance", Hyprlang::INT{300});

    HyprlandAPI::reloadConfig();

    HyprlandAPI::addNotification(PHANDLE, "[hyprsnap] Initialized successfully!", CHyprColor{0.2, 1.0, 0.2, 1.0}, 5000);

    return {"hyprsnap", "A plugin to add aerosnap functionality to floating windows.", "elviosak", "0.1"};
}

APICALL EXPORT void PLUGIN_EXIT() {
    //
    g_pGlobalState.reset();
}
