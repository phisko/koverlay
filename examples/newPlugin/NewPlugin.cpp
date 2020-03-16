#include "imgui.h"
#include "framework.hpp"

static const char * getName() {
	return "NewPlugin";
}

static void imguiFunction() {
	// can use g_scale to properly scale child windows and other elements
	if (ImGui::Begin("NewPlugin", &PLUGIN_ENABLED)) {
	}
	ImGui::End();
}
