#include "registerTypes.h"

#include "components/SelectedComponent.hpp"
#include "components/ShaderComponent.hpp"
#include "components/SkeletonComponent.hpp"
#include "components/TransformComponent.hpp"
#include "components/SkyBoxComponent.hpp"
#include "components/SpriteComponent.hpp"

void registerComponents3(kengine::EntityManager & em) {
	registerComponents<
		kengine::SelectedComponent,
		kengine::GBufferShaderComponent,
		kengine::LightingShaderComponent,
		kengine::PostProcessShaderComponent,
		kengine::ShadowCubeShaderComponent,
		kengine::ShadowMapShaderComponent,
		kengine::DepthCubeComponent,
		kengine::DepthMapComponent,
		kengine::SkeletonComponent,
		kengine::ModelSkeletonComponent,
		kengine::TransformComponent3f,
		kengine::SkyBoxComponent,
		kengine::SpriteComponent2D,
		kengine::SpriteComponent3D
	>(em);
}
