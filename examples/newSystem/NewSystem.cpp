#include "EntityManager.hpp"
#include "Export.hpp"

#include "helpers/PluginHelper.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/ImGuiComponent.hpp"
#include "imgui.h"

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
