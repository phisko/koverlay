#pragma once

#include "System.hpp"

class ClosestPlaneSystem : public kengine::System<ClosestPlaneSystem> {
public:
	ClosestPlaneSystem(kengine::EntityManager & em);

private:
	kengine::EntityManager & _em;
};
