#include "helpers/registerTypeHelper.hpp"
#include "data/InstanceComponent.hpp"

namespace types{
	void registerkengineInstanceComponent() noexcept {
		kengine::registerComponents<kengine::InstanceComponent>();

	}
}