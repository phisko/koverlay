#include "OpenGLSpritesSystem.hpp"

#include "EntityManager.hpp"
#include "SpritesShader.hpp"
#include "components/ModelComponent.hpp"
#include "components/TextureModelComponent.hpp"
#include "components/TextureLoaderComponent.hpp"
#include "components/ShaderComponent.hpp"
#include "components/GraphicsComponent.hpp"

#include "stb_image.h"

namespace kengine {
	OpenGLSpritesSystem::OpenGLSpritesSystem(kengine::EntityManager & em) : System(em), _em(em) {
		onLoad("");
	}

	void OpenGLSpritesSystem::onLoad(const char *) noexcept {
		_em += [this](kengine::Entity & e) {
			e += kengine::makeGBufferShaderComponent<SpritesShader>(_em);
		};
	}

	void OpenGLSpritesSystem::handle(kengine::packets::RegisterEntity p) {
		if (!p.e.has<GraphicsComponent>())
			return;

		auto & layer = p.e.get<GraphicsComponent>().getLayer("main");
		const auto & file = layer.appearance;

		for (const auto &[e, model, tex] : _em.getEntities<ModelComponent, TextureModelComponent>())
			if (model.file == file) {
				layer.model = e.id;
				return;
			}

		int width, height, components;
		const auto data = stbi_load(file.c_str(), &width, &height, &components, 0);
		if (data == nullptr)
			return; // Not supported image type

		_em += [&](Entity & e) {
			layer.model = e.id;

			auto & comp = e.attach<ModelComponent>();
			comp.file = file;

			TextureLoaderComponent textureLoader; {
				textureLoader.textureID = &e.attach<TextureModelComponent>().texture;

				textureLoader.data = data;
				textureLoader.width = width;
				textureLoader.height = height;
				textureLoader.components = components;

				textureLoader.free = stbi_image_free;
			} e += textureLoader;
		};
	}
}
