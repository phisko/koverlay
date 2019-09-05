#include "ImGuiLuaSystem.hpp"
#include "EntityManager.hpp"
#include "components/ImGuiComponent.hpp"
#include "packets/AddImGuiTool.hpp"
#include "systems/LuaSystem.hpp"

static float * g_scale;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

ImGuiLuaSystem::ImGuiLuaSystem(kengine::EntityManager & em) : System(em), _em(em) {
	extern lua_State * lState;
	extern void LoadImguiBindings();

	auto & state = em.getSystem<kengine::LuaSystem>().getState();
	lState = state;
	LoadImguiBindings();


	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
			state["IMGUI_SCALE"] = getScale();

			putils::Directory d("scripts");

			d.for_each([&](const putils::Directory::File & f) {
				const auto view = std::string_view(f.name);
				const auto dot = view.find_last_of('.');
				if (!f.isDirectory && dot != std::string_view::npos && view.substr(dot) == ".lua")
					state.script_file(f.fullPath.c_str());
			});

			// bool * enabled;
			// for (const auto & name : names)
			// 	send(kengine::packets::AddImGuiTool{ name, *enabled });
		});
	};
}

void ImGuiLuaSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}
