#pragma once

#include "System.hpp"

class NewSystem : public kengine::System<NewSystem> {
public:
	NewSystem(kengine::EntityManager & em);

private:
	kengine::EntityManager & _em;
};
