#pragma once

#include <algorithm>
#include <functional>

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#ifdef PSE_TGUI
#include <TGUI/Gui.hpp>
#include <TGUI/Widget.hpp>
#endif

#include <unordered_map>
#include "ViewItem.hpp"

//
// Phiste SFML Engine
//
namespace pse {
    class Engine {
        // Constructor
    public:
        Engine(size_t screenWidth, size_t screenHeight,
               const std::string & screenName, int sfWindowStyle = sf::Style::Close,
               float refreshRate = 120) noexcept;

        // Destructor
    public:
        ~Engine() = default;

        // Items getters and setters
    public:
        void addItem(ViewItem & item, size_t height = 0) { _items.push_back(std::make_pair(&item, height)); }

		void removeItem(const ViewItem & item) {
			const auto it = std::find_if(_items.begin(), _items.end(),
				[&item](auto & p) { return p.first == &item; });
			if (it != _items.end())
				_items.erase(it);
		}

        void setItemHeight(const ViewItem & item, std::size_t height) {
            auto it = std::find_if(_items.begin(), _items.end(),
                                   [&item](auto & p) { return p.first == &item; }
            );
            if (it != _items.end())
                it->second = height;
        }

    public:
        void addView(const std::string & name, const sf::Vector2f & center, const sf::Vector2f & size, size_t height) {
            _views.emplace(name, std::make_pair(sf::View(center, size), height));
        }

        bool hasView(const std::string & name) const noexcept { return _views.find(name) != _views.end(); }

        void removeView(const std::string & name) {
            const auto it = _views.find(name);
            if (it != _views.end())
                _views.erase(it);
        }

        void setViewHeight(const std::string & name, size_t height) { _views[name].second = height; }

        const sf::View & getView(const std::string & name) const { return _views.at(name).first; }
        sf::View & getView(const std::string & name) noexcept { return _views[name].first; }

        // Update the engine
        // clear: whether or not the screen should be splashed black
    public:
        void update(bool clear = true, const std::function<void()> & preDisplay = nullptr) noexcept;

        // Poll event
    public:
        bool pollEvent(sf::Event & e) noexcept {
            bool ret = _window.pollEvent(e);

#ifdef PSE_TGUI
            if (ret)
                _tgui.handleEvent(e);
#endif

            return ret;
        }

        // Splash a color on screen
    public:
        void displayColor(const sf::Color & color) noexcept { _window.clear(color); }

    private:
        void drawViews() noexcept;
        void drawItems() noexcept;

        // Access sf::RenderWindow
    public:
        sf::RenderWindow & getRenderWindow() noexcept { return _window; }

        // Attributes
    private:
        sf::RenderWindow _window;
        std::vector<std::pair<ViewItem *, size_t>> _items;
        std::unordered_map<std::string, std::pair<sf::View, size_t>> _views;
        putils::Timer _refreshTimer;

#ifdef PSE_TGUI
        public:
            template<typename T, typename = std::enable_if_t<std::is_base_of<tgui::Widget, T>::value>>
            void addItem(const std::shared_ptr<T> &widget, const std::string & name = "")
            { _tgui.add(widget, name); }

            template<typename T, typename = std::enable_if_t<std::is_base_of<tgui::Widget, T>::value>>
            bool removeItem(const std::shared_ptr<T> &widget)
            { return _tgui.remove(widget); }

            template<typename T, typename = std::enable_if_t<std::is_base_of<tgui::Widget, T>::value>>
            typename T::Ptr get(std::string_view name, bool recursive = false) const
            { return _tgui.get(name, recursive); }

            tgui::Gui &getGui() noexcept { return _tgui; }
        private:
            tgui::Gui _tgui;
#endif

        // Coplien
    public:
        Engine(const Engine &) = delete;

        Engine & operator=(const Engine &) = delete;
    };
}
