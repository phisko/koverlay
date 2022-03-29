#include "helpers/registerTypeHelper.hpp"
#include "data/AdjustableComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkengineAdjustableComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::AdjustableComponent'");
		kengine::registerComponents<kengine::AdjustableComponent>();

	}
}