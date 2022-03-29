#include "helpers/registerTypeHelper.hpp"
#include "data/PythonComponent.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkenginePythonComponent() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::PythonComponent'");
		kengine::registerComponents<kengine::PythonComponent>();

	}
}