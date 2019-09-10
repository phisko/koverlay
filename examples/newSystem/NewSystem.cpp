#include "NewSystem.hpp"

#include "EntityManager.hpp"
#include "Export.hpp"

#include "packets/AddImGuiTool.hpp"
#include "components/ImGuiComponent.hpp"
#include "imgui.h"

EXPORT kengine::ISystem * getSystem(kengine::EntityManager & em) {
	return new NewSystem(em);
}

static float * g_scale = nullptr;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

NewSystem::NewSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "NewSystem", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&em] {
				if (!display)
					return;

				if (ImGui::Begin("NewSystem"), &display) {
				}
				ImGui::End();
		});
	};
}

void NewSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}