#pragma once

#include "opengl/Program.hpp"
#include "Entity.hpp"

#include "components/ShaderComponent.hpp"

namespace kengine {
	class EntityManager;
	struct DirLightComponent;
	struct SpotLightComponent;
}

namespace kengine::Shaders {
	class ShadowMap : public ShadowMapShader {
	public:
		ShadowMap(kengine::EntityManager & em);

		void init(size_t firstTextureID, size_t screenWidth, size_t screenHeight, GLuint gBufferFBO) override;
		void run(const Parameters & params) override {}
		void run(kengine::Entity & e, DirLightComponent & light, const putils::Point3f & pos, size_t screenWidth, size_t screenHeight) override;
		void run(kengine::Entity & e, SpotLightComponent & light, const putils::Point3f & pos, size_t screenWidth, size_t screenHeight) override;

	private:
		template<typename T>
		void runImpl(kengine::Entity & e, T & light, const putils::Point3f & pos, size_t screenWidth, size_t screenHeight);

		kengine::EntityManager & _em;

	public:
		GLint proj;
		GLint view;
		GLint model;

		pmeta_get_attributes(
			pmeta_reflectible_attribute(&ShadowMap::proj),
			pmeta_reflectible_attribute(&ShadowMap::view),
			pmeta_reflectible_attribute(&ShadowMap::model)
		);
	};

}