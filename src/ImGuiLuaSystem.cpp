#include "ImGuiLuaSystem.hpp"
#include "EntityManager.hpp"

#include "data/ImGuiComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/LuaStateComponent.hpp"
#include "data/NameComponent.hpp"

#include "Directory.hpp"

static kengine::EntityManager * g_em;
static sol::state * g_state;

// declarations
static void initBindings();
static void runScripts();
//
kengine::EntityCreator * ImGuiLuaSystem(kengine::EntityManager & em) {
	g_em = &em;

	return [](kengine::Entity & system) {
		initBindings();

		system += kengine::ImGuiComponent([&] {
			runScripts();
		});
	};
}

static void initBindings() {
	extern lua_State * lState;
	extern void LoadImguiBindings();

	for (const auto & [e, state] : g_em->getEntities<kengine::LuaStateComponent>()) {
		lState = *state.state;
		LoadImguiBindings();

		g_state = state.state;
	}
}

// declarations
static void runScript(const char * script);
//
static void runScripts() {
	putils::Directory d("scripts");

	d.for_each([&](const putils::Directory::File & f) {
		const auto view = std::string_view(f.name);
		const auto dot = view.find_last_of('.');
		if (!f.isDirectory && dot != std::string_view::npos && view.substr(dot) == ".lua")
			runScript(f.fullPath.c_str());
	});
}

// declarations
static kengine::Entity getEntityForScript(const char * script);
//
static void runScript(const char * script) {
	auto e = getEntityForScript(script);
	auto & tool = e.get<kengine::ImGuiToolComponent>();

	if (!tool.enabled)
		return;

	(*g_state)["TOOL_ENABLED"] = tool.enabled;
	try {
		g_state->script_file(script);
	}
	catch (const std::exception & e) {
		std::cerr << e.what() << std::endl;
	}
	tool.enabled = (*g_state)["TOOL_ENABLED"];
}

static kengine::Entity getEntityForScript(const char * script) {
	static std::unordered_map<std::string, kengine::Entity::ID> ids;

	const auto it = ids.find(script);
	if (it != ids.end())
		return g_em->getEntity(it->second);

	return g_em->createEntity([&](kengine::Entity & e) {
		ids[script] = e.id;
		try {
			g_state->script_file(script);
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << std::endl;
		}
		e += kengine::ImGuiToolComponent{ (*g_state)["TOOL_ENABLED"] };
		const std::string name = (*g_state)["TOOL_NAME"];
		e += kengine::NameComponent{ name };
	});
}
