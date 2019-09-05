#pragma once

static bool PLUGIN_ENABLED; // Used to know if window is open
static const char * getName();
static void imguiFunction(float scale);

#if defined(__unix__) || defined(__APPLE__)
# define EXPORT extern "C"
#endif
#ifdef _WIN32
# define EXPORT extern "C" __declspec(dllexport)
#endif

EXPORT const char * getNameAndEnabled(bool ** outEnabled) {
	*outEnabled = &PLUGIN_ENABLED;
	return getName();
}

struct ImGuiContext;
extern ImGuiContext * GImGui;

EXPORT void drawImGui(ImGuiContext & context, float scale) {
	if (!PLUGIN_ENABLED)
		return;
	GImGui = &context;

	imguiFunction(scale);
}
