#include "EntityManager.hpp"
#include "Export.hpp"

#include "helpers/PluginHelper.hpp"
#include "helpers/SortHelper.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/ImGuiComponent.hpp"
#include "imgui.h"

EXPORT void loadKenginePlugin(kengine::EntityManager & em) {
	kengine::PluginHelper::initPlugin(em);

	em += [&](kengine::Entity & e) {
		e += kengine::NameComponent{ "Controller" };

		auto & tool = e.attach<kengine::ImGuiToolComponent>();
		tool.enabled = true;

		e += kengine::ImGuiComponent([&] {
			if (!tool.enabled)
				return;

			if (ImGui::Begin("Koverlay", &tool.enabled)) {
				const auto sorted = kengine::SortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>(em);
				for (const auto & [e, name, tool] : sorted)
					ImGui::Checkbox(name->name, &tool->enabled);
			}
			ImGui::End();
		});
	};
}
