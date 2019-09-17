#include "registerTypes.h"

#include "components/AdjustableComponent.hpp"
#include "components/AnimationComponent.hpp"
#include "components/BehaviorComponent.hpp"
#include "components/CameraComponent.hpp"
#include "components/DebugGraphicsComponent.hpp"
#include "components/GodRaysComponent.hpp"
#include "components/GraphicsComponent.hpp"
#include "components/HighlightComponent.hpp"
#include "components/ImGuiComponent.hpp"
#include "components/InputComponent.hpp"

void registerComponents1(kengine::EntityManager & em) {
	registerComponents<
		kengine::AdjustableComponent,
		kengine::AnimFilesComponent,
		kengine::AnimListComponent,
		kengine::AnimationComponent,
		kengine::BehaviorComponent,
		kengine::CameraComponent3f,
		kengine::DebugGraphicsComponent,
		kengine::GodRaysComponent,
		kengine::GraphicsComponent,
		kengine::HighlightComponent,
		kengine::ImGuiComponent,
		kengine::InputComponent
	>(em);
}
