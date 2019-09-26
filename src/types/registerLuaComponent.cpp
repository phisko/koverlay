#include "registerTypes.hpp"
#include "components/LuaComponent.hpp"

void registerLuaComponent(kengine::EntityManager & em) {
	registerComponents<
		kengine::LuaComponent,
		kengine::LuaTableComponent
	>(em);

	registerTypes<
		kengine::LuaComponent::script, kengine::LuaComponent::script_vector
	>(em);
}
