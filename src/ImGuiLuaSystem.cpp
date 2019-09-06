#include "ImGuiLuaSystem.hpp"
#include "EntityManager.hpp"
#include "components/ImGuiComponent.hpp"
#include "packets/AddImGuiTool.hpp"
#include "systems/LuaSystem.hpp"

static float * g_scale;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

struct ToolInfo {
	std::string name;
	bool enabled;
};

static ToolInfo & getToolInfo(const std::string & scriptName, bool & shouldRegisterTool) {
	static std::unordered_map<std::string, ToolInfo *> scriptToolMap;

	ToolInfo * toolInfo = nullptr;

	if (scriptToolMap.find(scriptName) == scriptToolMap.end()) {
		toolInfo = new ToolInfo{ "", true };
		scriptToolMap[scriptName] = toolInfo;
		shouldRegisterTool = true;
	}
	else
		toolInfo = scriptToolMap[scriptName];

	return *toolInfo;
}

static void runScript(const std::string & scriptName, kengine::EntityManager & em, sol::state & state) {
	bool shouldRegisterTool = false;
	auto & toolInfo = getToolInfo(scriptName, shouldRegisterTool);

	if (toolInfo.enabled) {
		state["TOOL_ENABLED"] = true;
		state.script_file(scriptName);
		toolInfo.enabled = state["TOOL_ENABLED"];

		if (shouldRegisterTool) {
			toolInfo.name = state["TOOL_NAME"];
			em.send(kengine::packets::AddImGuiTool{ toolInfo.name.c_str(), toolInfo.enabled });
		}
	}
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
					runScript(f.fullPath.c_str(), em, state);
			});
		});
	};
}

void ImGuiLuaSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}
