#include "kengine.hpp"
#include "Export.hpp"

#include "helpers/pluginHelper.hpp"
#include "helpers/sortHelper.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "functions/Execute.hpp"
#include "imgui.h"

EXPORT void loadKenginePlugin(void * state) noexcept {
	kengine::pluginHelper::initPlugin(state);

	kengine::entities += [&](kengine::Entity & e) noexcept {
		e += kengine::NameComponent{ "Controller" };

		auto & tool = e.attach<kengine::ImGuiToolComponent>();
		tool.enabled = true;

		e += kengine::functions::Execute{ [&](float deltaTime) noexcept {
			if (!tool.enabled)
				return;

			if (ImGui::Begin("Koverlay", &tool.enabled)) {
				const auto sorted = kengine::sortHelper::getNameSortedEntities<0, kengine::ImGuiToolComponent>();
				for (const auto & [e, name, tool] : sorted)
					ImGui::Checkbox(name->name, &tool->enabled);
			}
			ImGui::End();
		} };
	};
}
