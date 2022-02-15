#include "kengine.hpp"
#include "Export.hpp"

#include "helpers/pluginHelper.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/ImGuiScaleComponent.hpp"

#include "functions/Execute.hpp"

#include "imgui.h"

// can use this function to properly scale child windows and other elements
static float getScale() noexcept;

EXPORT void loadKenginePlugin(void * state) noexcept {
	kengine::pluginHelper::initPlugin(state);

	kengine::entities += [&](kengine::Entity & e) {
		e += kengine::NameComponent{ "New System" };

		auto & tool = e.attach<kengine::ImGuiToolComponent>();
		tool.enabled = false;

		e += kengine::functions::Execute{ [&](float deltaTime) noexcept {
			if (!tool.enabled)
				return;

			if (ImGui::Begin("NewSystem", &tool.enabled)) {
			}
			ImGui::End();
		}};
	};
}

static float getScale() noexcept {
	float ret = 1.f;
	for (const auto & [e, scale] : kengine::entities.with<kengine::ImGuiScaleComponent>())
		ret *= scale.scale;
	return ret;
}