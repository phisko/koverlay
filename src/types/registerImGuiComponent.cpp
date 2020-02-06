#include "registerTypes.hpp"
#include "data/ImGuiComponent.hpp"

void registerImGuiComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::ImGuiComponent
	>(em);
}
