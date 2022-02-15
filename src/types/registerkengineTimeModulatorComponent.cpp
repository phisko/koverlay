#include "helpers/registerTypeHelper.hpp"
#include "data/TimeModulatorComponent.hpp"

namespace types{
	void registerkengineTimeModulatorComponent() noexcept {
		kengine::registerComponents<kengine::TimeModulatorComponent>();

	}
}