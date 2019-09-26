#include "registerTypes.hpp"
#include "components/ImGuiComponent.hpp"

void registerImGuiComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::ImGuiComponent
	>(em);
}
