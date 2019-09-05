#include "ImGuiPluginSystem.hpp"
#include "EntityManager.hpp"
#include "components/ImGuiComponent.hpp"
#include "packets/AddImGuiTool.hpp"

static float * g_scale;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

ImGuiPluginSystem::ImGuiPluginSystem(kengine::EntityManager & em) : System(em), _em(em) {
	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
			static putils::PluginManager pm;

			bool * enabled;
			const auto names = pm.rescanDirectoryWithReturn<64, const char *>("plugins", "getNameAndEnabled", &enabled);
			for (const auto & name : names)
				send(kengine::packets::AddImGuiTool{ name, *enabled });

			pm.execute("drawImGui", *GImGui, getScale());
		});
	};
}

void ImGuiPluginSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}
