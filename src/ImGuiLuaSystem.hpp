#pragma once

#include "System.hpp"
#include "packets/ImGuiScale.hpp"

class ImGuiLuaSystem : public kengine::System<ImGuiLuaSystem, kengine::packets::ImGuiScale> {
public:
	ImGuiLuaSystem(kengine::EntityManager & em);

	void handle(const kengine::packets::ImGuiScale & p) const;

private:
	kengine::EntityManager & _em;
};
