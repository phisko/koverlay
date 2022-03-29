#include "helpers/registerTypeHelper.hpp"
#include "functions/Execute.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkenginefunctionsExecute() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::functions::Execute'");
		kengine::registerComponents<kengine::functions::Execute>();

	}
}