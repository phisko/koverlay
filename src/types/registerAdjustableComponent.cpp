#include "registerTypes.hpp"
#include "data/AdjustableComponent.hpp"

void registerAdjustableComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::AdjustableComponent
	>(em);
}