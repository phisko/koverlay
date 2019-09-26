#include "registerTypes.hpp"
#include "components/PyComponent.hpp"

void registerPyComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::PyComponent
	>(em);

	registerTypes<
		kengine::PyComponent::script, kengine::PyComponent::script_vector
	>(em);
}
