#include "registerTypes.hpp"
#include "data/PyComponent.hpp"

void registerPyComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::PyComponent
	>(em);

	registerTypes<
		kengine::PyComponent::script, kengine::PyComponent::script_vector
	>(em);
}
