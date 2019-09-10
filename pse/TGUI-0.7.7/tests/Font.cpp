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
#include <TGUI/Font.hpp>

TEST_CASE("[Font]") {
    sf::Font font1;
    auto font2 = std::make_shared<sf::Font>();

    REQUIRE(tgui::Font().getFont() == nullptr);
    REQUIRE(tgui::Font(nullptr).getFont() == nullptr);
    REQUIRE(tgui::Font(font1).getFont() != nullptr);
    REQUIRE(tgui::Font(font2).getFont() == font2);
    REQUIRE(tgui::Font("resources/DroidSansArmenian.ttf").getFont() != nullptr);
}
