#ifndef WIN32
static_assert(false, "Only implemented on Windows for now");
#endif

#include <optional>

#include <GLFW/glfw3.h>

#include <windowsx.h>
#define GLFW_EXPOSE_NATIVE_WIN32
# include <GLFW/glfw3native.h>
#undef GLFW_EXPOSE_NATIVE_WIN32

// putils
#include "go_to_bin_dir.hpp"
#include "PluginManager.hpp"
#include "on_scope_exit.hpp"

// kengine systems
#include "systems/glfw/GLFWSystem.hpp"
#include "systems/imgui_tool/ImGuiToolSystem.hpp"
#include "systems/imgui_prompt/ImGuiPromptSystem.hpp"
#include "systems/imgui_adjustable/ImGuiAdjustableSystem.hpp"
#include "systems/lua/LuaSystem.hpp"
#include "systems/opengl/OpenGLSystem.hpp"
#include "systems/python/PythonSystem.hpp"

// kengine data
#include "data/AdjustableComponent.hpp"
#include "data/CommandLineComponent.hpp"
#include "data/GLFWWindowComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/NameComponent.hpp"
#include "data/WindowComponent.hpp"

// kengine functions
#include "functions/Execute.hpp"

// kengine helpers
#include "helpers/mainLoop.hpp"
#include "helpers/sortHelper.hpp"

// systems
#include "ImGuiPluginSystem.hpp"
#include "ImGuiLuaSystem.hpp"

// src
#include "types/registerTypes.hpp"

#include "command_line_arguments.hpp"

static GLFWwindow * g_window;
static HHOOK g_hook;
static WNDPROC g_prevWndProc;

// Command-line arguments
struct Options {
	std::optional<float> scale;
    bool showWindow = false;
};
#define refltype Options
putils_reflection_info{
	putils_reflection_attributes(
		putils_reflection_attribute(scale),
        putils_reflection_attribute(showWindow,
            putils_reflection_metadata("help", "Show the debug window")
        )
	);
};
#undef refltype

namespace {
    struct impl {
        static void run(int ac, const char ** av) noexcept {
            kengine::init();
            types::registerTypes();

            const auto args = putils::toArgumentVector(ac, av);
            const auto options = putils::parseArguments<Options>(args, "Koala Overlay: a framework for ImGui tools");

            kengine::entities += [&](kengine::Entity &e) {
                e += kengine::CommandLineComponent{ args };
                e += kengine::WindowComponent{
                    .name = "Koverlay",
                    .size = {1, 1}
                };
            };

            addSystems();

            if (!options.showWindow)
                ShowWindow(GetConsoleWindow(), SW_HIDE);
            if (options.scale)
                setScale(*options.scale);

            const auto _ = setupWindow();
            loadPlugins();

            kengine::mainLoop::run();
        }

        static void addSystems() noexcept {
            kengine::entities += kengine::OpenGLSystem();
            kengine::entities += kengine::GLFWSystem();
            kengine::entities += kengine::LuaSystem();
            kengine::entities += kengine::PythonSystem();
            kengine::entities += kengine::ImGuiAdjustableSystem();
            kengine::entities += kengine::ImGuiPromptSystem();
            kengine::entities += kengine::ImGuiToolSystem();
            kengine::entities += ImGuiPluginSystem();
            kengine::entities += ImGuiLuaSystem();
        }

        static void setScale(float scale) noexcept {
            for (const auto[e, adjustable]: kengine::entities.with<kengine::AdjustableComponent>()) {
                if (adjustable.section != "ImGui")
                    continue;
                for (auto &val: adjustable.values)
                    if (val.name == "Scale") {
                        auto &storage = val.floatStorage;
                        *storage.ptr = scale;
                        storage.value = scale;
                    }
            }
        }

        static void loadPlugins() noexcept {
            putils::PluginManager pm;
            pm.rescanDirectory("plugins", "loadKenginePlugin", kengine::getState());
        }

#define MY_SYSTEM_TRAY_MESSAGE (WM_APP + 1) // arbitrary value between WP_APP and 0xBFFF

        static putils::OnScopeExit<std::function<void()>> setupWindow() noexcept {
            for (const auto[e, execute]: kengine::entities.with<kengine::functions::Execute>())
                execute(0.f); // Makes OpenGLSystem create window
            for (const auto[e, window]: kengine::entities.with<kengine::GLFWWindowComponent>()) {
                g_window = window.window.get();
                glfwHideWindow(g_window);
            }

            NOTIFYICONDATA nid;
            nid.cbSize = sizeof(nid);
            nid.uID = (UINT) std::hash<const char *>()("koala overlay");
            nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
            nid.hIcon = (HICON) LoadImage(nullptr, "resources/koala.ico", IMAGE_ICON, 0, 0,
                                          LR_DEFAULTSIZE | LR_LOADFROMFILE);
            nid.hWnd = glfwGetWin32Window(g_window);
            nid.uCallbackMessage = MY_SYSTEM_TRAY_MESSAGE;
            strcpy_s(nid.szTip, "Koverlay");

            Shell_NotifyIconA(NIM_ADD, &nid);

#ifdef GWL_WNDPROC
            g_prevWndProc = (WNDPROC)SetWindowLongPtr(nid.hWnd, GWL_WNDPROC, (LONG_PTR)&wndProc);
#else
            g_prevWndProc = (WNDPROC) SetWindowLongPtr(nid.hWnd, GWLP_WNDPROC, (LONG_PTR) &wndProc);
#endif

            g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookCallback, nullptr, 0);
            const std::function<void()> release = [] {
                UnhookWindowsHookEx(g_hook);
            };
            return putils::onScopeExit(release);
        }

        static inline bool g_enabled = true;

        struct ToolSave {
            bool enabled = false;
        };

        static LRESULT wndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
            if (umsg == MY_SYSTEM_TRAY_MESSAGE) {
                switch (lParam) {
                    case WM_RBUTTONUP:
                        showContextMenu(true);
                        break;
                    case WM_LBUTTONDBLCLK:
                        toggleAllTools();
                        break;
                    default:
                        break;
                }
            } else if (umsg == WM_COMMAND) { // In context menu
                const auto id = LOWORD(wParam);
                size_t i = 0;
                bool found = false;
                auto sorted = kengine::sortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>();
                for (auto &[e, name, tool]: sorted) {
                    if (id != i) {
                        ++i;
                        continue;
                    }

                    if (g_enabled)
                        tool->enabled = !tool->enabled;
                    else {
                        auto &save = e.attach<ToolSave>();
                        save.enabled = !save.enabled;
                    }

                    found = true;
                    break;
                }

                if (!found) { // "Exit"
                    kengine::stopRunning();
                    if (!g_enabled)
                        toggleAllTools();
                }
            } else
                showContextMenu(false);

            return CallWindowProc(g_prevWndProc, hwnd, umsg, wParam, lParam);
        }

        static void showContextMenu(bool shown) noexcept {
            static HMENU hMenu;

            DestroyMenu(hMenu);
            if (!shown)
                return;
            glfwFocusWindow(g_window);
            hMenu = CreatePopupMenu();

            size_t i = 0;
            const auto sorted = kengine::sortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>();
            for (const auto &[e, name, tool]: sorted) {
                AppendMenu(hMenu, MF_STRING, i, name->name);
                ++i;
            }
            AppendMenu(hMenu, MF_STRING, i, "Exit");

            POINT curPoint;
            GetCursorPos(&curPoint);

            const auto hWnd = glfwGetWin32Window(g_window);
            TrackPopupMenu(hMenu, 0, curPoint.x, curPoint.y, 0, hWnd, nullptr);
        }

        static LRESULT __stdcall hookCallback(int nCode, WPARAM wParam, LPARAM lParam) {
            static bool altPressed = false;
            static bool disablePressed = false;

            const auto ret = [&] { return CallNextHookEx(g_hook, nCode, wParam, lParam); };

            if (nCode != HC_ACTION)
                return ret();

            const auto kbd = (KBDLLHOOKSTRUCT *) lParam;
            const bool keyDown = wParam == WM_SYSKEYDOWN;

            switch (kbd->vkCode) {
                case VK_LMENU:
                    altPressed = keyDown;
                    break;
                case 'Q':
                    disablePressed = keyDown;
                    break;
            }

            if (altPressed && disablePressed)
                toggleAllTools();

            return ret();
        }

        static void toggleAllTools() noexcept {
            glfwFocusWindow(g_window);

            for (auto[e, name, tool]: kengine::entities.with<kengine::NameComponent, kengine::ImGuiToolComponent>())
                if (g_enabled) {
                    e += ToolSave{tool.enabled};
                    tool.enabled = false;
                } else
                    tool.enabled = e.attach<ToolSave>().enabled;

            static bool first = true; // Don't know why, first time this is called it leaves one of the tools open
            if (first && g_enabled) {
                first = false;
                for (auto[e, name, tool]: kengine::entities.with<kengine::NameComponent, kengine::ImGuiToolComponent>())
                    if (tool.enabled) {
                        e += ToolSave{tool.enabled};
                        tool.enabled = false;
                    }
            }

            g_enabled = !g_enabled;
        }
    };
}

int main(int ac, const char **av) {
	putils::goToBinDir(av[0]);
	impl::run(ac, av);
	return 0;
}
