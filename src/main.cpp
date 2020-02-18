#ifndef WIN32
static_assert(false, "Only implemented on Windows for now");
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windowsx.h>
#define GLFW_EXPOSE_NATIVE_WIN32
# include <GLFW/glfw3native.h>
#undef GLFW_EXPOSE_NATIVE_WIN32

#include "go_to_bin_dir.hpp"
#include "PluginManager.hpp"

#include "systems/LuaSystem.hpp"
#include "systems/PySystem.hpp"
#include "systems/ImGuiToolSystem.hpp"
#include "systems/ImGuiPromptSystem.hpp"
#include "systems/opengl/OpenGLSystem.hpp"

#include "ImGuiPluginSystem.hpp"
#include "ImGuiLuaSystem.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/WindowComponent.hpp"
#include "data/GLFWWindowComponent.hpp"
#include "functions/Execute.hpp"

#include "helpers/MainLoop.hpp"
#include "helpers/SortHelper.hpp"

static kengine::EntityManager * g_em;
static GLFWwindow * g_window;
static HHOOK g_hook;
static WNDPROC g_prevWndProc;

// declarations
static void addSystems();
static void setupWindow();
static void loadPlugins();
//
int main(int, char **av) {
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	putils::goToBinDir(av[0]);

	kengine::EntityManager em;
	g_em = &em;

	em += [](kengine::Entity & e) {
		e += kengine::WindowComponent{
			"Koverlay",
			{ 1, 1 }
		};
	};

	addSystems();
	setupWindow();
	loadPlugins();

	extern void registerTypes(kengine::EntityManager &);
	registerTypes(em);

	kengine::MainLoop::run(em);

	UnhookWindowsHookEx(g_hook);

	return 0;
}

static void addSystems() {
	*g_em += kengine::OpenGLSystem(*g_em);
	*g_em += kengine::LuaSystem(*g_em);
	*g_em += kengine::PySystem(*g_em);
	// *g_em += kengine::ImGuiAdjustableSystem(*g_em);
	*g_em += kengine::ImGuiToolSystem(*g_em);
	*g_em += ImGuiPluginSystem(*g_em);
	*g_em += ImGuiLuaSystem(*g_em);
	*g_em += ImGuiPromptSystem(*g_em);
}

static void loadPlugins() {
	putils::PluginManager pm;
	pm.rescanDirectory("plugins", "loadKenginePlugin", *g_em);
}

// declarations
#define MY_SYSTEM_TRAY_MESSAGE (WM_APP + 1) // arbitrary value between WP_APP and 0xBFFF
static LRESULT wndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam);
static LRESULT __stdcall hookCallback(int nCode, WPARAM wParam, LPARAM lParam);
//
static void setupWindow() {
	for (const auto & [e, execute] : g_em->getEntities<kengine::functions::Execute>())
		execute(0.f); // Makes OpenGLSystem create window
	for (const auto & [e, window] : g_em->getEntities<kengine::GLFWWindowComponent>()) {
		g_window = window.window;
		glfwHideWindow(g_window);
	}

	NOTIFYICONDATA nid;
	nid.cbSize = sizeof(nid);
	nid.uID = std::hash<const char *>()("koala overlay");
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hIcon = (HICON)LoadImage(nullptr, "resources/koala.ico", IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	nid.hWnd = glfwGetWin32Window(g_window);
	nid.uCallbackMessage = MY_SYSTEM_TRAY_MESSAGE;
	strcpy_s(nid.szTip, "Koverlay");

	Shell_NotifyIconA(NIM_ADD, &nid);

#ifdef GWL_WNDPROC
	g_prevWndProc = (WNDPROC)SetWindowLongPtr(nid.hWnd, GWL_WNDPROC, (LONG_PTR)&wndProc);
#else
	g_prevWndProc = (WNDPROC)SetWindowLongPtr(nid.hWnd, GWLP_WNDPROC, (LONG_PTR)&wndProc);
#endif

	g_hook = SetWindowsHookEx(WH_KEYBOARD_LL, hookCallback, nullptr, 0);
}

static bool g_enabled = true;
struct ToolSave {
	bool enabled = false;
};

// declarations
static void showContextMenu(bool shown);
static void toggleAllTools();
//
static LRESULT wndProc(HWND hwnd, UINT umsg, WPARAM wParam, LPARAM lParam) {
	if (umsg == MY_SYSTEM_TRAY_MESSAGE) {
		if (lParam == WM_RBUTTONUP)
			showContextMenu(true);
	}
	else if (umsg == WM_COMMAND) { // In context menu
		const auto id = LOWORD(wParam);
		size_t i = 0;
		bool found = false;
		auto sorted = kengine::SortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>(*g_em);
		for (auto & [e, name, tool] : sorted) {
			if (id != i) {
				++i;
				continue;
			}

			if (g_enabled)
				tool->enabled = !tool->enabled;
			else {
				auto & save = e.attach<ToolSave>();
				save.enabled = !save.enabled;
			}

			found = true;
			break;
		}

		if (!found) { // "Exit"
			g_em->running = false;
			if (!g_enabled)
				toggleAllTools();
		}
	}
	else
		showContextMenu(false);

	return CallWindowProc(g_prevWndProc, hwnd, umsg, wParam, lParam);
}

static void showContextMenu(bool shown) {
	static HMENU hMenu;

	DestroyMenu(hMenu);
	if (!shown)
		return;
	glfwFocusWindow(g_window);
	hMenu = CreatePopupMenu();

	size_t i = 0;
	const auto sorted = kengine::SortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>(*g_em);
	for (const auto & [e, name, tool] : sorted) {
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

	const auto kbd = (KBDLLHOOKSTRUCT *)lParam;
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

static void toggleAllTools() {
	glfwFocusWindow(g_window);

	for (auto & [e, name, tool] : g_em->getEntities<kengine::NameComponent, kengine::ImGuiToolComponent>())
		if (g_enabled) {
			e += ToolSave{ tool.enabled };
			tool.enabled = false;
		}
		else
			tool.enabled = e.attach<ToolSave>().enabled;

	static bool first = true; // Don't know why, first time this is called it leaves one of the tools open
	if (first && g_enabled) {
		first = false;
		for (auto & [e, name, tool] : g_em->getEntities<kengine::NameComponent, kengine::ImGuiToolComponent>())
			if (tool.enabled) {
				e += ToolSave{ tool.enabled };
				tool.enabled = false;
			}
	}

	g_enabled = !g_enabled;
}
