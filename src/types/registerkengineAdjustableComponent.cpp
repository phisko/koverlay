#include "helpers/registerTypeHelper.hpp"
#include "data/AdjustableComponent.hpp"

namespace types{
	void registerkengineAdjustableComponent() noexcept {
		kengine::registerComponents<kengine::AdjustableComponent>();

	}
}