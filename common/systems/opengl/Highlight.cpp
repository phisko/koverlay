#include "Highlight.hpp"
#include "EntityManager.hpp"

#include "components/HighlightComponent.hpp"

#include "shaders/shaders.hpp"
#include "helpers/ShaderHelper.hpp"

static const auto frag = R"(
#version 330

uniform vec3 viewPos;
uniform vec2 screenSize;

uniform sampler2D gposition;
uniform sampler2D gnormal;
uniform sampler2D gentityID;

uniform float entityID;
uniform vec4 highlightColor;
uniform float intensity;

out vec4 outColor;

void main() {
	outColor = vec4(0.0);

   	vec2 texCoord = gl_FragCoord.xy / screenSize;

    vec4 worldPos = texture(gposition, texCoord);
	if (worldPos.w == 0) // Empty pixel
		return;

	vec3 normal = -texture(gnormal, texCoord).xyz;
	if (texture(gentityID, texCoord).x != entityID)
		return;

	float highlight = 1.0 - dot(normalize(normal), normalize(worldPos.xyz - viewPos)) + intensity;
	outColor = highlight * highlightColor;
}
)";

namespace kengine::Shaders {
	Highlight::Highlight(kengine::EntityManager & em)
		: Program(true, pmeta_nameof(Highlight)),
		_em(em)
	{
	}

	void Highlight::init(size_t firstTextureID, size_t screenWidth, size_t screenHeight, GLuint gBufferFBO) {
		initWithShaders<Highlight>(putils::make_vector(
			ShaderDescription{ src::Quad::vert, GL_VERTEX_SHADER },
			ShaderDescription{ frag, GL_FRAGMENT_SHADER }
		));
	}

	void Highlight::run(const Parameters & params) {
		use();

		ShaderHelper::Enable _(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		putils::gl::setUniform(viewPos, params.camPos);
		putils::gl::setUniform(screenSize, params.screenSize);

		for (const auto & [e, highlight] : _em.getEntities<HighlightComponent>()) {
			putils::gl::setUniform(entityID, (float)e.id);
			putils::gl::setUniform(highlightColor, highlight.color);
			putils::gl::setUniform(intensity, highlight.intensity * 2.f - 1.f); // convert from [0,1] to [-1,1]
			ShaderHelper::shapes::drawQuad();
		}
	}
}