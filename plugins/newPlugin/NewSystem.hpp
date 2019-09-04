#pragma once

#include "System.hpp"
#include "packets/ImGuiScale.hpp"

class NewSystem : public kengine::System<NewSystem> {
public:
	NewSystem(kengine::EntityManager & em);

	void handle(const kengine::packets::ImGuiScale & p) const;

private:
	kengine::EntityManager & _em;
};
