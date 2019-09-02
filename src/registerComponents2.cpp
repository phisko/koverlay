#include "registerTypes.h"

#include "components/LightComponent.hpp"
#include "components/LuaComponent.hpp"
#include "components/ModelComponent.hpp"
#include "components/ModelColliderComponent.hpp"
#include "components/NameComponent.hpp"
#include "components/PhysicsComponent.hpp"
#include "components/PyComponent.hpp"
#include "components/TextComponent.hpp"

void registerComponents2(kengine::EntityManager & em) {
	registerComponents<
		kengine::DirLightComponent,
		kengine::PointLightComponent,
		kengine::SpotLightComponent,
		kengine::LuaComponent,
		kengine::ModelComponent,
		kengine::ModelColliderComponent,
		kengine::NameComponent,
		kengine::PhysicsComponent,
		kengine::PyComponent,
		kengine::TextComponent2D,
		kengine::TextComponent3D
	>(em);

	registerTypes<
		kengine::ModelColliderComponent::Collider,
		kengine::TextComponent::string
	>(em);
}
