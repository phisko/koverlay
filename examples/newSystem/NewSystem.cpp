#include "EntityManager.hpp"
#include "Export.hpp"

#include "helpers/PluginHelper.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/ImGuiComponent.hpp"

#include "functions/GetImGuiScale.hpp"

#include "imgui.h"

// can use this function to properly scale child windows and other elements
static float getScale(kengine::EntityManager & em);

EXPORT void loadKenginePlugin(kengine::EntityManager & em) {
	kengine::PluginHelper::initPlugin(em);

	em += [&](kengine::Entity & e) {
		e += kengine::NameComponent{ "New System" };

		auto & tool = e.attach<kengine::ImGuiToolComponent>();
		tool.enabled = false;

		e += kengine::ImGuiComponent([&] {
			if (!tool.enabled)
				return;

			if (ImGui::Begin("NewSystem", &tool.enabled)) {
			}
			ImGui::End();
		});
	};
}

static float getScale(kengine::EntityManager & em) {
	for (const auto & [e, getScale] : em.getEntities<kengine::functions::GetImGuiScale>())
		return getScale();
	return 1.f;
}
