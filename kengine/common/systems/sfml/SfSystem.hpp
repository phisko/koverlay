#pragma once


#include "System.hpp"
#include "components/TransformComponent.hpp"
#include "components/GraphicsComponent.hpp"
#include "packets/RegisterAppearance.hpp"
#include "packets/RemoveEntity.hpp"
#include "packets/RegisterEntity.hpp"

#include "TGUI/TGUI.hpp"
#include "TGUI/Widget.hpp"
#include "pse/Engine.hpp"
#include "SfComponent.hpp"

#include "json.hpp"

namespace kengine {
    class EntityManager;

    class SfSystem : public kengine::System<SfSystem,
            packets::RegisterEntity, packets::RemoveEntity,
            packets::RegisterAppearance> {
    public:
        SfSystem(kengine::EntityManager & em);

    public:
        void execute() final;
        void handle(const packets::RegisterEntity & p);
        void handle(const packets::RemoveEntity & p);

		pse::Engine & getEngine() { return _engine; }
		void setShouldClear(bool shouldClear) { _shouldClear = shouldClear; }

    private:
		void attachDebug(Entity go);
		void attachGUI(Entity go);
		void attachNormal(Entity go);
		void attachLayer(SfComponent & comp, const kengine::GraphicsComponent::Layer & layer, const putils::Rect3f & boundingBox);

    public:
        void handle(const packets::RegisterAppearance & p) noexcept;

    private:
        std::unique_ptr<pse::Sprite> getResource(const std::string & appearance);
        void registerLuaFunctions() noexcept;
        void handleEvents() noexcept;
        void updateCameras() noexcept;
        void updateDrawables();
        bool updateDebug(EntityView go, pse::ViewItem & item);
        void updateObject(EntityView go, pse::ViewItem & item, const GraphicsComponent::Layer & layer, bool fixedSize);
        void updateGUIElement(EntityView go) noexcept;
        void updateTransform(pse::ViewItem & item, const kengine::TransformComponent3f & transform, const GraphicsComponent::Layer & layer, bool fixedSize) noexcept;

	private:
		putils::json _config;
		putils::Point2f _screenSize;
		putils::Point2f _tileSize;
		bool _fullScreen;
		bool _shouldClear = true;

		// Config parsers
	private:
		putils::Point2f parseSize(const std::string & propertyName, const putils::Point2f & _default);
		bool parseBool(const std::string & propertyName, bool _default);

		// Helpers
    private:
		sf::Vector2f toWorldPos(const putils::Point3f & pos) const {
			return { (float)(pos.x * _tileSize.x), (float)(-pos.z * _tileSize.y) };
		}

		sf::Vector2f toWorldSize(const putils::Point3f & pos) const {
			return { (float)(pos.x * _tileSize.x), (float)(pos.z * _tileSize.y) };
		}

	private:
		pse::Engine _engine;
		kengine::EntityManager & _em;

		struct GUIElement {
			std::shared_ptr<tgui::Widget> frame = nullptr;
			std::shared_ptr<tgui::Label> label = nullptr;
		};
		std::unordered_map<Entity::ID, GUIElement> _guiElements;

	private:
		sf::Clock _deltaClock;

	private:
		std::unordered_map<std::string, std::string> _appearances;

		// Input
	private:
		std::unordered_map<sf::Keyboard::Key, bool> _pressedKeys;
		std::unordered_map<sf::Mouse::Button, bool> _pressedButtons;
	};
}
