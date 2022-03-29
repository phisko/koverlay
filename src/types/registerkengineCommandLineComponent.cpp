#include "helpers/registerTypeHelper.hpp"
#include "data/CommandLineComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkengineCommandLineComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::CommandLineComponent'");
		kengine::registerComponents<kengine::CommandLineComponent>();

	}
}