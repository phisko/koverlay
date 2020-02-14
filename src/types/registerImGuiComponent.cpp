#include "registerTypes.hpp"
#include "data/ImGuiComponent.hpp"
#include "data/ImGuiToolComponent.hpp"

void registerImGuiComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::ImGuiComponent,
		kengine::ImGuiToolComponent
	>(em);
}
