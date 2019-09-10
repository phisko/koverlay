#include "Debug.hpp"
#include "EntityManager.hpp"

#include "components/DebugGraphicsComponent.hpp"
#include "components/TransformComponent.hpp"

#include "helpers/ShaderHelper.hpp"
#include "shaders/shaders.hpp"

static const char * vert = R"(
#version 330

layout (location = 0) in vec3 position;

uniform mat4 proj;
uniform mat4 view;
uniform mat4 model;
uniform vec3 viewPos;

out vec4 WorldPosition;
out vec3 EyeRelativePos;

void main() {
	WorldPosition = model * vec4(position, 1.0);
	EyeRelativePos = WorldPosition.xyz - viewPos;
    gl_Position = proj * view * WorldPosition;
}
)";


static const char * frag = R"(
#version 330

in vec4 WorldPosition;
in vec3 EyeRelativePos;

uniform float entityID;
uniform vec4 color;

layout (location = 0) out vec4 gposition;
layout (location = 1) out vec3 gnormal;
layout (location = 2) out vec4 gcolor;
layout (location = 3) out float gentityID;

void applyTransparency(float alpha);

void main() {
	applyTransparency(color.a);

    gposition = WorldPosition;
    gnormal = -normalize(cross(dFdy(EyeRelativePos), dFdx(EyeRelativePos)));
	gcolor = vec4(color.rgb, 1.0); // don't apply lighting
	gentityID = entityID;
}
)";

namespace kengine::Shaders {
	Debug::Debug(kengine::EntityManager & em)
		: Program(false, pmeta_nameof(Debug)),
		_em(em)
	{
	}

	void Debug::init(size_t firstTextureID, size_t screenWidth, size_t screenHeight, GLuint gBufferFBO) {
		initWithShaders<Debug>(putils::make_vector(
			ShaderDescription{ vert, GL_VERTEX_SHADER },
			ShaderDescription{ frag, GL_FRAGMENT_SHADER },
			ShaderDescription{ src::ApplyTransparency::frag, GL_FRAGMENT_SHADER }
		));
	}

	void Debug::run(const Parameters & params) {
		use();

		putils::gl::setUniform(this->view, params.view);
		putils::gl::setUniform(this->proj, params.proj);
		putils::gl::setUniform(this->viewPos, params.camPos);

		for (const auto &[e, debug, transform] : _em.getEntities<kengine::DebugGraphicsComponent, kengine::TransformComponent3f>()) {
			putils::gl::setUniform(this->color, debug.color);
			putils::gl::setUniform(this->entityID, (float)e.id);

			if (debug.debugType == DebugGraphicsComponent::Line) {
				glm::mat4 model(1.f);
				model = glm::translate(model, ShaderHelper::toVec(transform.boundingBox.topLeft));
				model = glm::rotate(model, transform.roll, { 0.f, 0.f, 1.f });
				model = glm::rotate(model, transform.yaw, { 0.f, 1.f, 0.f });
				model = glm::rotate(model, transform.pitch, { 1.f, 0.f, 0.f });
				putils::gl::setUniform(this->model, model);

				ShaderHelper::shapes::drawLine({ 0.f, 0.f, 0.f }, ShaderHelper::toVec(debug.offset.topLeft));
			}
			else if (debug.debugType == DebugGraphicsComponent::Sphere || debug.debugType == DebugGraphicsComponent::Box) {
				glm::mat4 model(1.f);
				model = glm::translate(model, ShaderHelper::toVec(transform.boundingBox.topLeft + debug.offset.topLeft));
				model = glm::rotate(model, transform.roll, { 0.f, 0.f, 1.f });
				model = glm::rotate(model, transform.yaw, { 0.f, 1.f, 0.f });
				model = glm::rotate(model, transform.pitch, { 1.f, 0.f, 0.f });
				model = glm::scale(model, ShaderHelper::toVec(transform.boundingBox.size * debug.offset.size));
				putils::gl::setUniform(this->model, model);

				if (debug.debugType == DebugGraphicsComponent::Box)
					ShaderHelper::shapes::drawCube();
				else
					ShaderHelper::shapes::drawSphere();
			}
			else if (debug.debugType == DebugGraphicsComponent::Text) {
				// TODO
			}
			else
				assert(!"Unsupported DebugGraphicsComponent type"); // Unsupported type
		}
	}
}