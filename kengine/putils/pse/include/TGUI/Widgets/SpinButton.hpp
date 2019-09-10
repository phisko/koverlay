/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2015 Bruno Van de Velde (vdv_b@tgui.eu)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#ifndef TGUI_SPIN_BUTTON_HPP
#define TGUI_SPIN_BUTTON_HPP


#include <TGUI/Widgets/ClickableWidget.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    class SpinButtonRenderer;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Spin button widget
    ///
    /// Signals:
    ///     - ValueChanged (The value of the spin button has changed)
    ///         * Optional parameter int: New value
    ///         * Uses Callback member 'value'
    ///
    ///     - Inherited signals from ClickableWidget
    ///
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API SpinButton : public ClickableWidget
    {
    public:

        typedef std::shared_ptr<SpinButton> Ptr; ///< Shared widget pointer
        typedef std::shared_ptr<const SpinButton> ConstPtr; ///< Shared constant widget pointer


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Default constructor
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        SpinButton();


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Makes a copy of another spin button
        ///
        /// @param spinButton  The other spin button
        ///
        /// @return The new spin button
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static SpinButton::Ptr copy(SpinButton::ConstPtr spinButton);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the renderer, which gives access to functions that determine how the widget is displayed
        ///
        /// @return Reference to the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        std::shared_ptr<SpinButtonRenderer> getRenderer() const
        {
            return std::static_pointer_cast<SpinButtonRenderer>(m_renderer);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Set the position of the widget
        ///
        /// This function completely overwrites the previous position.
        /// See the move function to apply an offset based on the previous position instead.
        /// The default position of a transformable widget is (0, 0).
        ///
        /// @param position  New position
        ///
        /// @see move, getPosition
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setPosition(const Layout2d& position) override;
        using Transformable::setPosition;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the size of the spin button.
        ///
        /// @param size  The new size of the spin button
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setSize(const Layout2d& size) override;
        using Transformable::setSize;


        //////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the full size of the spin button
        ///
        /// The size returned by this function includes the borders.
        ///
        /// @return Full size of the spin button
        ///
        //////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual sf::Vector2f getFullSize() const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Sets a minimum value.
        ///
        /// @param minimum  The new minimum value
        ///
        /// When the value is too small then it will be changed to this minimum.
        /// The default minimum value is 0.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setMinimum(int minimum);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Sets a maximum value.
        ///
        /// @param maximum  The new maximum value
        ///
        /// When the value is too big then it will be changed to this maximum.
        /// The default maximum value is 10.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setMaximum(int maximum);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the current value.
        ///
        /// @param value  The new value
        ///
        /// The value can't be smaller than the minimum or bigger than the maximum.
        /// The default value is 0.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setValue(int value);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the minimum value.
        ///
        /// @return The current minimum value
        ///
        /// The default minimum value 0.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int getMinimum() const
        {
            return m_minimum;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the maximum value.
        ///
        /// @return The current maximum value
        ///
        /// The default maximum value 10.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int getMaximum() const
        {
            return m_maximum;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the current value.
        ///
        /// @return The current value
        ///
        /// The default value is 0.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        int getValue() const
        {
            return m_value;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes whether the spin button lies vertical or horizontal (arrows above or next to each other).
        ///
        /// @param verticalScroll  Does the spin button lie vertically?
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setVerticalScroll(bool verticalScroll);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns whether the spin button lies vertical or horizontal (arrows above or next to each other).
        ///
        /// @return Does the spin button lie vertically?
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        bool getVerticalScroll() const
        {
            return m_verticalScroll;
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the opacity of the widget.
        ///
        /// @param opacity  The opacity of the widget. 0 means completely transparent, while 1 (default) means fully opaque.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setOpacity(float opacity) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Returns the distance between the position where the widget is drawn and where the widget is placed
        ///
        /// This is basically the width and height of the optional borders drawn around widgets.
        ///
        /// @return Offset of the widget
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual sf::Vector2f getWidgetOffset() const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void leftMousePressed(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void leftMouseReleased(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void mouseMoved(float x, float y) override;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @internal
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void widgetFocused() override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Reload the widget
        ///
        /// @param primary    Primary parameter for the loader
        /// @param secondary  Secondary parameter for the loader
        /// @param force      Try to only change the looks of the widget and not alter the widget itself when false
        ///
        /// @throw Exception when the connected theme could not create the widget
        ///
        /// When primary is an empty string the built-in white theme will be used.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void reload(const std::string& primary = "", const std::string& secondary = "", bool force = false) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Makes a copy of the widget
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual Widget::Ptr clone() const override
        {
            return std::make_shared<SpinButton>(*this);
        }


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Draws the widget on the render target.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        // Is the spin button draw vertically (arrows on top of each other)?
        bool m_verticalScroll = true;

        int m_minimum = 0;
        int m_maximum = 10;
        int m_value = 0;

        // On which arrow is the mouse?
        bool m_mouseHoverOnTopArrow = false;
        bool m_mouseDownOnTopArrow = false;

        friend class SpinButtonRenderer;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    class TGUI_API SpinButtonRenderer : public WidgetRenderer, public WidgetBorders
    {
    public:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Constructor
        ///
        /// @param spinButton  The spin button that is connected to the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        SpinButtonRenderer(SpinButton* spinButton) : m_spinButton{spinButton} {}


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change a property of the renderer
        ///
        /// @param property  The property that you would like to change
        /// @param value     The new serialized value that you like to assign to the property
        ///
        /// @throw Exception when deserialization fails or when the widget does not have this property.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setProperty(std::string property, const std::string& value) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change a property of the renderer
        ///
        /// @param property  The property that you would like to change
        /// @param value     The new value that you like to assign to the property.
        ///                  The ObjectConverter is implicitly constructed from the possible value types.
        ///
        /// @throw Exception for unknown properties or when value was of a wrong type.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual void setProperty(std::string property, ObjectConverter&& value) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Retrieve the value of a certain property
        ///
        /// @param property  The property that you would like to retrieve
        ///
        /// @return The value inside a ObjectConverter object which you can extract with the correct get function or
        ///         an ObjectConverter object with type ObjectConverter::Type::None when the property did not exist.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual ObjectConverter getProperty(std::string property) const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Get a map with all properties and their values
        ///
        /// @return Property-value pairs of the renderer
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual std::map<std::string, ObjectConverter> getPropertyValuePairs() const override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the background color of the arrows.
        ///
        /// @param color  New background color
        ///
        /// This color will overwrite the color for both normal and hover states.
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /// @see setBackgroundColorNormal
        /// @see setBackgroundColorHover
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBackgroundColor(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the background color of the arrows in the normal state (mouse not on spin button).
        ///
        /// @param color  New background color
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBackgroundColorNormal(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the background color of the arrows in the hover state (mouse standing on top of the spin button).
        ///
        /// @param color  New background color
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBackgroundColorHover(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the arrows.
        ///
        /// @param color  New arrow color
        ///
        /// This color will overwrite the color for both normal and hover states.
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /// @see setArrowColorNormal
        /// @see setArrowColorHover
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowColor(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the arrows in the normal state (mouse not on spin button).
        ///
        /// @param color  New arrow color
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowColorNormal(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the arrows in the hover state (mouse standing on top of the spin button).
        ///
        /// @param color  New arrow color
        ///
        /// Note that this color is ignored when an up and down arrow image were set.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowColorHover(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the color of the borders that are optionally drawn around the arrows.
        ///
        /// @param color  New border color
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setBorderColor(const Color& color);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the image that is used as the up arrow
        ///
        /// @param texture  The new arrow texture
        ///
        /// When this image and the down image are set, the color properties will be ignored.
        /// Pass an empty texture to unset the image, in this case the color properties will be used again.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowUpTexture(const Texture& texture);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the image that is used as the down arrow
        ///
        /// @param texture  The new arrow texture
        ///
        /// When this image and the up image are set, the color properties will be ignored.
        /// Pass an empty texture to unset the image, in this case the color properties will be used again.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowDownTexture(const Texture& texture);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the image that is used as the up arrow when the mouse is on top of this arrow
        ///
        /// @param texture  The new hover arrow texture
        ///
        /// The hover texture is ignored if no normal texture was set.
        /// Pass an empty texture to unset the image.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowUpHoverTexture(const Texture& texture);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Change the image that is used as the up arrow when the mouse is on top of this arrow
        ///
        /// @param texture  The new hover arrow texture
        ///
        /// The hover texture is ignored if no normal texture was set.
        /// Pass an empty texture to unset the image.
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setArrowDownHoverTexture(const Texture& texture);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Changes the space that is placed between the arrows.
        ///
        /// This space will use the BorderColor as color.
        ///
        /// When images are used, this space is scaled together with the size of the images
        ///
        /// @param space  New space between the arrows
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void setSpaceBetweenArrows(float space);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Draws the widget on the render target.
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        void draw(sf::RenderTarget& target, sf::RenderStates states) const;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private:

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // Makes a copy of the renderer
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        virtual std::shared_ptr<WidgetRenderer> clone(Widget* widget) override;


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    protected:

        SpinButton* m_spinButton;

        float     m_spaceBetweenArrows = 2;

        Texture   m_textureArrowUpNormal;
        Texture   m_textureArrowUpHover;
        Texture   m_textureArrowDownNormal;
        Texture   m_textureArrowDownHover;

        sf::Color m_backgroundColorNormal;
        sf::Color m_backgroundColorHover;
        sf::Color m_arrowColorNormal;
        sf::Color m_arrowColorHover;

        sf::Color m_borderColor;


        friend class SpinButton;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_SPIN_BUTTON_HPP
