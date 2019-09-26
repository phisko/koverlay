#include "registerTypes.hpp"
#include "components/AdjustableComponent.hpp"

void registerAdjustableComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::AdjustableComponent
	>(em);
}