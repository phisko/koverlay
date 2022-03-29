#include "helpers/registerTypeHelper.hpp"
#include "data/WindowComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkengineWindowComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::WindowComponent'");
		kengine::registerComponents<kengine::WindowComponent>();

	}
}