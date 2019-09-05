#include "ImGuiPluginSystem.hpp"
#include "EntityManager.hpp"
#include "components/ImGuiComponent.hpp"

static float * g_scale;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

ImGuiPluginSystem::ImGuiPluginSystem(kengine::EntityManager & em) : System(em), _em(em) {
	em += [](kengine::Entity & e) {
		e += kengine::ImGuiComponent([] {
			static putils::PluginManager pm("plugins");
			pm.execute("drawImGui", getScale());
		});
	};
}

void ImGuiPluginSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}
