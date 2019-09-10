#pragma once

#ifndef KENGINE_GRAPHICS_STRING_MAX_LENGTH
# define KENGINE_GRAPHICS_STRING_MAX_LENGTH 64
#endif

#ifndef KENGINE_GRAPHICS_MAX_LAYERS
# define KENGINE_GRAPHICS_MAX_LAYERS 8
#endif

#include "string.hpp"
#include "vector.hpp"
#include "Entity.hpp"
#include "Color.hpp"

namespace kengine {
    class GraphicsComponent : public putils::Reflectible<GraphicsComponent> {
    public:
		static constexpr char stringName[] = "GraphicsComponentString";
		using string = putils::string<KENGINE_GRAPHICS_STRING_MAX_LENGTH, stringName>;

    public:
        GraphicsComponent(const char * appearance = nullptr) {
			if (appearance != nullptr)
				addLayer("main", appearance);
        }

		struct Layer : public putils::Reflectible<Layer> {
			string name;
			string appearance;
			kengine::Entity::ID model = kengine::Entity::INVALID_ID; // Entity which had a ModelComponent

			putils::NormalizedColor color = { 1.f, 1.f, 1.f, 1.f };

			pmeta_get_class_name(GraphicsComponentLayer);
			pmeta_get_attributes(
				pmeta_reflectible_attribute(&Layer::name),
				pmeta_reflectible_attribute(&Layer::appearance),
				pmeta_reflectible_attribute(&Layer::model),

				pmeta_reflectible_attribute(&Layer::color)
			);
			pmeta_get_methods();
			pmeta_get_parents();
		};
		using layer_vector = putils::vector<Layer, KENGINE_GRAPHICS_MAX_LAYERS>;
		layer_vector layers;

		Layer & addLayer(const char * name, const char * appearance) {
			auto & ret = layers.emplace_back();
			ret.name = name;
			ret.appearance = appearance;
			return layers.back();
		}

		void removeLayer(const char * name) {
			const auto it = std::find_if(layers.begin(), layers.end(), [&name](auto && layer) { return layer.name == name; });
			if (it != layers.end())
				layers.erase(it);
		}

		bool hasLayer(const char * name) const {
			return std::find_if(layers.begin(), layers.end(), [&name](auto && layer) { return layer.name == name; }) != layers.end();
		}

		Layer & getLayer(const char * name) {
			const auto it = std::find_if(layers.begin(), layers.end(), [&name](auto && layer) { return layer.name == name; });
			return *it;
		}

		// Different name is important because of reflection (need different names, otherwise function pointer is unresolvable)
		const Layer & getLayerConst(const char * name) const {
			const auto it = std::find_if(layers.begin(), layers.end(), [&name](auto && layer) { return layer.name == name; });
			return *it;
		}

        /*
         * Reflectible
         */

    public:
        pmeta_get_class_name(GraphicsComponent);
        pmeta_get_attributes(
                pmeta_reflectible_attribute(&GraphicsComponent::layers)
        );
		pmeta_get_methods(
			pmeta_reflectible_attribute(&GraphicsComponent::addLayer),
			pmeta_reflectible_attribute(&GraphicsComponent::removeLayer),
			pmeta_reflectible_attribute(&GraphicsComponent::getLayer),
			pmeta_reflectible_attribute(&GraphicsComponent::hasLayer)
		);
		pmeta_get_parents();
    };
}
