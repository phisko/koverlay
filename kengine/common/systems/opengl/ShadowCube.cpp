#include "ShadowCube.hpp"
#include "shaders/shaders.hpp"

#include "components/LightComponent.hpp"
#include "components/DefaultShadowComponent.hpp"
#include "components/ModelComponent.hpp"
#include "components/GraphicsComponent.hpp"

#include "helpers/ShaderHelper.hpp"
#include "helpers/LightHelper.hpp"

namespace kengine::Shaders {
	void ShadowCube::init(size_t firstTextureID, size_t screenWidth, size_t screenHeight, GLuint gBufferFBO) {
		initWithShaders<ShadowCube>(putils::make_vector(
			ShaderDescription{ src::ProjViewModel::vert, GL_VERTEX_SHADER },
			ShaderDescription{ src::DepthCube::geom, GL_GEOMETRY_SHADER },
			ShaderDescription{ src::DepthCube::frag, GL_FRAGMENT_SHADER }
		));

		putils::gl::setUniform(proj, glm::mat4(1.f));
		putils::gl::setUniform(view, glm::mat4(1.f));
	}

	void ShadowCube::run(kengine::Entity & e, PointLightComponent & light, const putils::Point3f & pos, float radius, size_t screenWidth, size_t screenHeight) {
		if (!e.has<DepthCubeComponent>()) {
			auto & depthCube = e.attach<DepthCubeComponent>();

			glGenFramebuffers(1, &depthCube.fbo);
			ShaderHelper::BindFramebuffer __f(depthCube.fbo);

			glGenTextures(1, &depthCube.texture);
			glBindTexture(GL_TEXTURE_CUBE_MAP, depthCube.texture);
			for (size_t i = 0; i < 6; ++i)
				glTexImage2D((GLenum)(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i), 0, GL_DEPTH_COMPONENT, KENGINE_SHADOW_CUBE_SIZE, KENGINE_SHADOW_CUBE_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

			glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCube.texture, 0);
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glViewport(0, 0, KENGINE_SHADOW_CUBE_SIZE, KENGINE_SHADOW_CUBE_SIZE);
		glCullFace(GL_FRONT);

		const auto & depthCube = e.get<DepthCubeComponent>();
		ShaderHelper::BindFramebuffer __f(depthCube.fbo);
		ShaderHelper::Enable __e(GL_DEPTH_TEST);

		use();

		static struct {
			GLint shadowMatrixUniform;
			glm::vec3 target;
			glm::vec3 up;
		} directions[] = {
			{ -1, { 1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ -1, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
			{ -1, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
			{ -1, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
			{ -1, { 0.0f, 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
			{ -1, { 0.0f, 0.0f, -1.0f }, { 0.0f, -1.0f, 0.0f } }
		};

		const glm::vec3 vPos(pos.x, pos.y, pos.z);
		const auto proj = glm::perspective(glm::radians(90.f), (float)KENGINE_SHADOW_CUBE_SIZE / (float)KENGINE_SHADOW_CUBE_SIZE, SHADOW_MAP_NEAR_PLANE, SHADOW_MAP_FAR_PLANE);
		for (unsigned int i = 0; i < 6; ++i) {
			if (directions[i].shadowMatrixUniform == -1) {
				const putils::string<64> shadowMatrix("shadowMatrices[%d]", i);
				directions[i].shadowMatrixUniform = glGetUniformLocation(getHandle(), shadowMatrix.c_str());
				assert(directions[i].shadowMatrixUniform != -1);
			}
			putils::gl::setUniform(directions[i].shadowMatrixUniform, proj * glm::lookAt(vPos, vPos + directions[i].target, directions[i].up));
		}

		putils::gl::setUniform(lightPos, vPos);
		putils::gl::setUniform(farPlane, radius);

		for (const auto &[e, graphics, transform, shadow] : _em.getEntities<GraphicsComponent, TransformComponent3f, DefaultShadowComponent>()) {
			const auto & layer = graphics.getLayer("main");
			if (layer.model == kengine::Entity::INVALID_ID)
				continue;

			const auto & modelInfoEntity = _em.getEntity(layer.model);
			if (!modelInfoEntity.has<OpenGLModelComponent>() || !modelInfoEntity.has<ModelComponent>())
				continue;

			const auto & modelInfo = modelInfoEntity.get<ModelComponent>();
			const auto & openGL = modelInfoEntity.get<OpenGLModelComponent>();

			putils::gl::setUniform(this->model, ShaderHelper::getModelMatrix(modelInfo, transform));
			ShaderHelper::drawModel(openGL);
		}

		glViewport(0, 0, (GLsizei)screenWidth, (GLsizei)screenHeight);
		glCullFace(GL_BACK);
	}
}
