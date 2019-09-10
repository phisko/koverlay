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


#include <TGUI/Transformable.hpp>

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace tgui
{
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Transformable::Transformable(const Transformable& other)
    {
        Layout2d position = other.getPositionLayout();
        Layout2d size = other.getSizeLayout();

        if (other.m_position.x.getImpl()->operation == LayoutImpl::Operation::String)
            position.x = other.m_position.x.getImpl()->stringExpression;
        if (other.m_position.y.getImpl()->operation == LayoutImpl::Operation::String)
            position.y = other.m_position.y.getImpl()->stringExpression;

        if (other.m_size.x.getImpl()->operation == LayoutImpl::Operation::String)
            size.x = other.m_size.x.getImpl()->stringExpression;
        if (other.m_size.y.getImpl()->operation == LayoutImpl::Operation::String)
            size.y = other.m_size.y.getImpl()->stringExpression;

        setPosition(position);
        setSize(size);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Transformable& Transformable::operator=(const Transformable& other)
    {
        if (this != &other)
        {
            // We can only copy the layouts when they are strings
            Layout2d position = {other.getPosition()};
            Layout2d size = {other.getSize()};

            if (other.m_position.x.getImpl()->operation == LayoutImpl::Operation::String)
                position.x = other.m_position.x.getImpl()->stringExpression;
            if (other.m_position.y.getImpl()->operation == LayoutImpl::Operation::String)
                position.y = other.m_position.y.getImpl()->stringExpression;

            if (other.m_size.x.getImpl()->operation == LayoutImpl::Operation::String)
                size.x = other.m_size.x.getImpl()->stringExpression;
            if (other.m_size.y.getImpl()->operation == LayoutImpl::Operation::String)
                size.y = other.m_size.y.getImpl()->stringExpression;

            setPosition(position);
            setSize(size);
        }

        return *this;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Transformable::~Transformable()
    {
        m_position.x.getImpl()->attachedLayouts.erase(&m_position.x);
        m_position.y.getImpl()->attachedLayouts.erase(&m_position.y);
        m_size.x.getImpl()->attachedLayouts.erase(&m_size.x);
        m_size.y.getImpl()->attachedLayouts.erase(&m_size.y);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::setPosition(const Layout2d& position)
    {
        m_position.x.getImpl()->attachedLayouts.erase(&m_position.x);
        m_position.y.getImpl()->attachedLayouts.erase(&m_position.y);

        m_position = position;
        m_prevPosition = m_position.getValue();

        m_position.x.getImpl()->attachedLayouts.insert(&m_position.x);
        m_position.y.getImpl()->attachedLayouts.insert(&m_position.y);

        m_position.x.connectUpdateCallback([this](){ updatePosition(false); });
        m_position.y.connectUpdateCallback([this](){ updatePosition(false); });
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::move(const Layout2d& offset)
    {
        setPosition(m_position + offset);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::move(const Layout& x, const Layout& y)
    {
        setPosition({m_position.x + x, m_position.y + y});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::setSize(const Layout2d& size)
    {
        m_size.x.getImpl()->attachedLayouts.erase(&m_size.x);
        m_size.y.getImpl()->attachedLayouts.erase(&m_size.y);

        m_size = size;
        m_prevSize = m_size.getValue();

        m_size.x.getImpl()->attachedLayouts.insert(&m_size.x);
        m_size.y.getImpl()->attachedLayouts.insert(&m_size.y);

        m_size.x.connectUpdateCallback([this](){ updateSize(false); });
        m_size.y.connectUpdateCallback([this](){ updateSize(false); });
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::scale(const Layout2d& factors)
    {
        setSize({m_size.x * factors.x, m_size.y * factors.y});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::scale(const Layout& x, const Layout& y)
    {
        setSize({m_size.x * x, m_size.y * y});
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::updatePosition(bool forceUpdate)
    {
        if (forceUpdate)
            setPosition(m_position);
        else
        {
            if (m_prevPosition != m_position.getValue())
                setPosition(m_position);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void Transformable::updateSize(bool forceUpdate)
    {
        if (forceUpdate)
            setSize(m_size);
        else
        {
            if (m_prevSize != m_size.getValue())
                setSize(m_size);
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
