#include "imgui.h"
#include "framework.hpp"

static const char * getName() {
	return "NewPlugin";
}

static void imguiFunction() {
	if (ImGui::Begin("NewPlugin", &PLUGIN_ENABLED)) {
	}
	ImGui::End();
}
