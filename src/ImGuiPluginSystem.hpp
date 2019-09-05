#pragma once

#include "System.hpp"
#include "packets/ImGuiScale.hpp"

class ImGuiPluginSystem : public kengine::System<ImGuiPluginSystem, kengine::packets::ImGuiScale> {
public:
	ImGuiPluginSystem(kengine::EntityManager & em);

	void handle(const kengine::packets::ImGuiScale & p) const;

private:
	kengine::EntityManager & _em;
};
