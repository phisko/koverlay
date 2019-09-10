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


#ifndef TGUI_DATA_IO_HPP
#define TGUI_DATA_IO_HPP

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <TGUI/Config.hpp>
#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <map>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// @brief Parser and emitter for widget files
    /// @internal
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    class TGUI_API DataIO
    {
    public:
        struct ValueNode;

        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Widget file node
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        struct Node
        {
            Node* parent = nullptr;
            std::vector<std::shared_ptr<Node>> children;
            std::map<std::string, std::shared_ptr<ValueNode>> propertyValuePairs;
            std::string name;
        };


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Widget file value node
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        struct ValueNode
        {
            ValueNode(Node* p = nullptr, const std::string& v = "") : parent(p), value(v) {}

            Node* parent;
            std::string value;

            bool listNode = false;
            std::vector<std::string> valueList;
        };


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Parse a widget file
        ///
        /// @param stream  Stream containing the widget file
        ///
        /// @return Root node of the tree of nodes
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static std::shared_ptr<Node> parse(std::stringstream& stream);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        /// @brief Emit the widget file
        ///
        /// @param rootNode Root node of the tree of nodes that is to be converted to a string stream
        /// @param stream   Stream to which the widget file will be added
        ///
        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        static void emit(std::shared_ptr<Node> rootNode, std::stringstream& stream);


        /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private:

        static std::vector<std::string> convertNodesToLines(std::shared_ptr<Node> node);

        static std::string parseSection(std::stringstream& stream, std::shared_ptr<Node> node, const std::string& sectionName);

        static std::string parseKeyValue(std::stringstream& stream, std::shared_ptr<Node> node, const std::string& key);

        static std::string readLine(std::stringstream& stream);

        static std::string readWord(std::stringstream& stream);
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // TGUI_DATA_IO_HPP
