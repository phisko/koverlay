/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TGUI - Texus's Graphical User Interface
// Copyright (C) 2012-2017 Bruno Van de Velde (vdv_b@tgui.eu)
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

#include "Tests.hpp"
#include <TGUI/TGUI.hpp>

bool compareVector2f(sf::Vector2f left, sf::Vector2f right)
{
    return std::abs(left.x - right.x) < 0.0001f
        && std::abs(left.y - right.y) < 0.0001f;
}

TEST_CASE("[Animation]") {
    tgui::Button::Ptr widget = std::make_shared<tgui::Button>();
    widget->getRenderer()->setBorders({2, 2, 2, 2});
    widget->setPosition(30, 15);
    widget->setSize(120, 30);
    widget->setOpacity(0.9f);

    tgui::Panel::Ptr parent = std::make_shared<tgui::Panel>();
    parent->setSize(480, 360);
    parent->add(widget);

    SECTION("Show/Hide with effects") {
        SECTION("Widget becomes visible when effect starts") {
            widget->hide();
            REQUIRE(!widget->isVisible());
            widget->showWithEffect(tgui::ShowAnimationType::Scale, sf::milliseconds(250));
            REQUIRE(widget->isVisible());

            // Widget remains visible after animation has finished
            widget->update(sf::milliseconds(250));
            REQUIRE(widget->isVisible());
        }

        SECTION("Widget is still visible when effect starts") {
            REQUIRE(widget->isVisible());
            widget->hideWithEffect(tgui::ShowAnimationType::Fade, sf::milliseconds(250));
            REQUIRE(widget->isVisible());

            // Widget gets hidden after animation has finished
            widget->update(sf::milliseconds(250));
            REQUIRE(!widget->isVisible());
        }

        SECTION("Time can go past the animation end") {
            widget->showWithEffect(tgui::ShowAnimationType::SlideFromLeft, sf::milliseconds(250));
            widget->update(sf::milliseconds(500));
            REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
        }

        SECTION("showWithEffect") {
            SECTION("Fade") {
                widget->showWithEffect(tgui::ShowAnimationType::Fade, sf::milliseconds(300));
                REQUIRE(widget->getOpacity() == 0);
                widget->update(sf::milliseconds(100));
                REQUIRE(tgui::compareFloats(widget->getOpacity(), 0.3f));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getOpacity() == 0.9f);
            }

            SECTION("Scale") {
                widget->showWithEffect(tgui::ShowAnimationType::Scale, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(90, 30));
                REQUIRE(widget->getSize() == sf::Vector2f(0, 0));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {70, 25}));
                REQUIRE(compareVector2f(widget->getSize(), {40, 10}));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
            }

            SECTION("SlideFromLeft") {
                widget->showWithEffect(tgui::ShowAnimationType::SlideFromLeft, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(-124, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {-124.f+((124.f+30.f)/3.f), 15}));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            }

            SECTION("SlideFromTop") {
                widget->showWithEffect(tgui::ShowAnimationType::SlideFromTop, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, -34));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30, -34.f+((34.f+15.f)/3.f)}));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            }

            SECTION("SlideFromRight") {
                widget->showWithEffect(tgui::ShowAnimationType::SlideFromRight, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(482, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {482-((482-30)/3.f), 15}));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            }

            SECTION("SlideFromBottom") {
                widget->showWithEffect(tgui::ShowAnimationType::SlideFromBottom, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 362));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30, 362-((362-15)/3.f)}));
                widget->update(sf::milliseconds(200));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            }

            // The widget no longer changes after the animation is over
            widget->update(sf::milliseconds(100));
            REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
            REQUIRE(widget->getOpacity() == 0.9f);
        }

        SECTION("hideWithEffect") {
            SECTION("Fade") {
                widget->hideWithEffect(tgui::ShowAnimationType::Fade, sf::milliseconds(300));
                REQUIRE(widget->getOpacity() == 0.9f);
                widget->update(sf::milliseconds(100));
                REQUIRE(tgui::compareFloats(widget->getOpacity(), 0.6f));
            }

            SECTION("Scale") {
                widget->hideWithEffect(tgui::ShowAnimationType::Scale, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {50, 20}));
                REQUIRE(compareVector2f(widget->getSize(), {80, 20}));
            }

            SECTION("SlideToRight") {
                widget->hideWithEffect(tgui::ShowAnimationType::SlideToRight, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30+((482-30)/3.f), 15}));
            }

            SECTION("SlideToBottom") {
                widget->hideWithEffect(tgui::ShowAnimationType::SlideToBottom, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30, 15+((362-15)/3.f)}));
            }

            SECTION("SlideToLeft") {
                widget->hideWithEffect(tgui::ShowAnimationType::SlideToLeft, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30.f-((124.f+30.f)/3.f), 15}));
            }

            SECTION("SlideToTop") {
                widget->hideWithEffect(tgui::ShowAnimationType::SlideToTop, sf::milliseconds(300));
                REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
                widget->update(sf::milliseconds(100));
                REQUIRE(compareVector2f(widget->getPosition(), {30, 15.f-((34.f+15.f)/3.f)}));
            }

            // The widget is hidden but reset to its original values at the end of the animation
            REQUIRE(widget->isVisible());
            widget->update(sf::milliseconds(200));
            REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
            REQUIRE(widget->getOpacity() == 0.9f);
            REQUIRE(!widget->isVisible());

            // The widget no longer changes after the animation is over
            widget->update(sf::milliseconds(100));
            REQUIRE(widget->getPosition() == sf::Vector2f(30, 15));
            REQUIRE(widget->getSize() == sf::Vector2f(120, 30));
            REQUIRE(widget->getOpacity() == 0.9f);
            REQUIRE(!widget->isVisible());
        }

        // TODO: Add tests for simultaneous animations (tests for both same type and different types)
    }
}
