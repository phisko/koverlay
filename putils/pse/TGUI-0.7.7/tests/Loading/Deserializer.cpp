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

#include "../catch.hpp"
#include <TGUI/Loading/Deserializer.hpp>

using Type = tgui::ObjectConverter::Type;

TEST_CASE("[Deserializer]") {
    SECTION("deserialize font") {
        REQUIRE(tgui::Deserializer::deserialize(tgui::ObjectConverter::Type::Font, "resources/DroidSansArmenian.ttf").getFont() != nullptr);
        REQUIRE(tgui::Deserializer::deserialize(tgui::ObjectConverter::Type::Font, "\"resources/DroidSansArmenian.ttf\"").getFont() != nullptr);
        REQUIRE(tgui::Deserializer::deserialize(tgui::ObjectConverter::Type::Font, "nullptr").getFont() == nullptr);
        REQUIRE(tgui::Deserializer::deserialize(tgui::ObjectConverter::Type::Font, "null").getFont() == nullptr);
    }

    SECTION("deserialize color") {
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, ",,").getColor() == sf::Color(0, 0, 0));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "10, 20, 30").getColor() == sf::Color(10, 20, 30));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "40, 50, 60, 70").getColor() == sf::Color(40, 50, 60, 70));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "(80, 90, 100)").getColor() == sf::Color(80, 90, 100));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "(110, 120, 130, 140)").getColor() == sf::Color(110, 120, 130, 140));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "rgb(100, 0, 50)").getColor() == sf::Color(100, 0, 50));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "rgba(100, 0, 50, 200)").getColor() == sf::Color(100, 0, 50, 200));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "#123").getColor() == sf::Color(17, 34, 51));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "#4567").getColor() == sf::Color(68, 85, 102, 119));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "#89ABCD").getColor() == sf::Color(137, 171, 205));
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "#FEDCBA98").getColor() == sf::Color(254, 220, 186, 152));

        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "BLACK").getColor() == sf::Color::Black);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "White").getColor() == sf::Color::White);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "red").getColor() == sf::Color::Red);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "green").getColor() == sf::Color::Green);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "blue").getColor() == sf::Color::Blue);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "magenta").getColor() == sf::Color::Magenta);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "yellow").getColor() == sf::Color::Yellow);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "cyan").getColor() == sf::Color::Cyan);

        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, ""), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, "rgb(0,1)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, ",,,,"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, "#Hi"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, "#12345"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, "#123456789"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Color, "RandomString"), tgui::Exception);
    }

    SECTION("deserialize string") {
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\"").getString() == "");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"Just a string.\"").getString() == "Just a string.");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\\\\"").getString() == "\\");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\n\"").getString() == "\n");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\t\"").getString() == "\t");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\v\"").getString() == "\v");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\\"\"").getString() == "\"");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"a\\t\\\"str\\\"?\\nYES!\"").getString() == "a\t\"str\"?\nYES!");

        REQUIRE(tgui::Deserializer::deserialize(Type::String, "").getString() == "");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\\").getString() == "\\");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\\n").getString() == "\\n");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "str").getString() == "str");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\\m\"").getString() == "m");
        REQUIRE(tgui::Deserializer::deserialize(Type::String, "\"\n\"").getString() == "\n");
    }

    SECTION("deserialize number") {
        REQUIRE(tgui::Deserializer::deserialize(Type::Number, "0").getNumber() == 0);
        REQUIRE(tgui::Deserializer::deserialize(Type::Number, "1").getNumber() == 1);
        REQUIRE(tgui::Deserializer::deserialize(Type::Number, "2.25").getNumber() == 2.25);
    }

    SECTION("deserialize borders") {
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "1, 2, 3, 4").getBorders() == tgui::Borders(1, 2, 3, 4));
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "(5, 6, 7, 8)").getBorders() == tgui::Borders(5, 6, 7, 8));
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "9, 10").getBorders() == tgui::Borders(9, 10, 9, 10));
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "(11, 12)").getBorders() == tgui::Borders(11, 12, 11, 12));
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "13").getBorders() == tgui::Borders(13, 13, 13, 13));
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "(14)").getBorders() == tgui::Borders(14, 14, 14, 14));

        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Borders, "(1,2,3)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Borders, ",,,,"), tgui::Exception);
    }

    SECTION("deserialize texture") {
        tgui::Texture texture;

        texture = tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\"").getTexture();
        REQUIRE(texture.isLoaded());
        REQUIRE(texture.getData()->rect == sf::IntRect());
        REQUIRE(texture.getMiddleRect() == sf::IntRect(0, 0, 50, 50));

        texture = tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Part(0, 0, 25, 25)").getTexture();
        REQUIRE(texture.isLoaded());
        REQUIRE(texture.getData()->rect == sf::IntRect(0, 0, 25, 25));
        REQUIRE(texture.getMiddleRect() == sf::IntRect(0, 0, 25, 25));

        texture = tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Middle(10, 10, 30, 30)").getTexture();
        REQUIRE(texture.isLoaded());
        REQUIRE(texture.getData()->rect == sf::IntRect());
        REQUIRE(texture.getMiddleRect() == sf::IntRect(10, 10, 30, 30));

        texture = tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Part(0, 0, 40, 40) Middle(10, 10, 20, 20)").getTexture();
        REQUIRE(texture.isLoaded());
        REQUIRE(texture.getData()->rect == sf::IntRect(0, 0, 40, 40));
        REQUIRE(texture.getMiddleRect() == sf::IntRect(10, 10, 20, 20));

        texture = tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Middle(10, 10, 20, 10) Part(20, 10, 40, 30)").getTexture();
        REQUIRE(texture.isLoaded());
        REQUIRE(texture.getData()->rect == sf::IntRect(20, 10, 40, 30));
        REQUIRE(texture.getMiddleRect() == sf::IntRect(10, 10, 20, 10));

        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, ""), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "resources/image.png"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "resources/image.png\""), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" (0,0,0,0)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" xyz(0,0,0,0)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Part(0,1)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Middle(0,)"), tgui::Exception);
        REQUIRE_THROWS_AS(tgui::Deserializer::deserialize(Type::Texture, "\"resources/image.png\" Middle(10, 10, 20, 20"), tgui::Exception);
    }

    SECTION("custom deserialize function") {
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "rgb(10, 20, 30)").getColor() == sf::Color(10, 20, 30));
        auto oldFunc = tgui::Deserializer::getFunction(tgui::ObjectConverter::Type::Color);
        
        tgui::Deserializer::setFunction(Type::Color, [](const std::string&){ return tgui::ObjectConverter{sf::Color::Green}; });
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "rgb(10, 20, 30)").getColor() == sf::Color::Green);
        REQUIRE(tgui::Deserializer::deserialize(Type::Borders, "(50, 60, 70, 80)").getBorders() == tgui::Borders(50, 60, 70, 80));
        
        tgui::Deserializer::setFunction(tgui::ObjectConverter::Type::Color, oldFunc);
        REQUIRE(tgui::Deserializer::deserialize(Type::Color, "rgb(10, 20, 30)").getColor() == sf::Color(10, 20, 30));
    }
}
