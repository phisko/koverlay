#pragma once

#include "System.hpp"
#include "packets/ImGuiScale.hpp"

class ClosestPlaneSystem : public kengine::System<ClosestPlaneSystem, kengine::packets::ImGuiScale> {
public:
	ClosestPlaneSystem(kengine::EntityManager & em);
	~ClosestPlaneSystem();

	void execute() final;
	void handle(const kengine::packets::ImGuiScale & p) const;

private:
	kengine::EntityManager & _em;
};
