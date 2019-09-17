#include "go_to_bin_dir.hpp"

#include "systems/LuaSystem.hpp"
#include "systems/PySystem.hpp"
#include "systems/ImGuiAdjustableSystem.hpp"
#include "systems/ImGuiEntityEditorSystem.hpp"
#include "systems/ImGuiEntitySelectorSystem.hpp"
#include "systems/BehaviorSystem.hpp"
#include "systems/imgui_overlay/ImGuiOverlaySystem.hpp"

#include "ImGuiPluginSystem.hpp"
#include "ImGuiLuaSystem.hpp"

#include "imgui.h"

#include "registerTypes.h"

int main(int, char **av) {
	ShowWindow(GetConsoleWindow(), SW_HIDE);

	putils::goToBinDir(av[0]);

	kengine::EntityManager em(std::thread::hardware_concurrency());

	em.loadSystems<
		kengine::LuaSystem, kengine::PySystem,
		kengine::BehaviorSystem,
		kengine::ImGuiOverlaySystem,
		kengine::ImGuiAdjustableSystem,
		kengine::ImGuiEntityEditorSystem,
		kengine::ImGuiEntitySelectorSystem,
		ImGuiPluginSystem,
		ImGuiLuaSystem
	>("plugins");

	registerTypes(em);

	while (em.running) {
		em.execute();
		em.loadSystems("plugins");
	}
}
