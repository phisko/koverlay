#include "ImGuiPluginSystem.hpp"
#include "PluginManager.hpp"
#include "EntityManager.hpp"

#include "functions/GetImGuiScale.hpp"
#include "functions/Execute.hpp"

#include "data/NameComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/ImGuiComponent.hpp"

static kengine::EntityManager * g_em;
static putils::PluginManager pm;

struct ImGuiContext;
extern ImGuiContext * GImGui;

// declarations
static void execute(float deltaTime);
static void drawImGui();
//
kengine::EntityCreator * ImGuiPluginSystem(kengine::EntityManager & em) {
	g_em = &em;

	return [](kengine::Entity & system) {
		system += kengine::functions::Execute{ execute };
		system += kengine::ImGuiComponent( drawImGui );
	};
}

struct PluginEnabledPointerComponent {
	bool * enabled;
};

static void execute(float deltaTime) {
	bool * enabled;
	const auto names = pm.rescanDirectoryWithReturn<1, const char *>("plugins", "getNameAndEnabled", &enabled);
	for (const auto & name : names) {
		*g_em += [&](kengine::Entity & e) {
			e += kengine::NameComponent{ name };
			e += PluginEnabledPointerComponent{ enabled };
			e += kengine::ImGuiToolComponent{ *enabled };
		};
	}
}

static void drawImGui() {
	float scale = 1.f;
	for (const auto & [e, getScale] : g_em->getEntities<kengine::functions::GetImGuiScale>())
		scale = getScale();

	for (const auto & [e, enabled, tool] : g_em->getEntities<PluginEnabledPointerComponent, kengine::ImGuiToolComponent>())
		*enabled.enabled = tool.enabled;

	pm.execute("drawImGui", *GImGui, scale);

	for (const auto & [e, enabled, tool] : g_em->getEntities<PluginEnabledPointerComponent, kengine::ImGuiToolComponent>())
		tool.enabled = *enabled.enabled;

}