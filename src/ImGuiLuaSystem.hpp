#pragma once

#include "EntityCreator.hpp"
namespace kengine { class EntityManager; }

kengine::EntityCreator * ImGuiLuaSystem(kengine::EntityManager & em);
