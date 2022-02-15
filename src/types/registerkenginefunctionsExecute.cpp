#include "helpers/registerTypeHelper.hpp"
#include "functions/Execute.hpp"

namespace types{
	void registerkenginefunctionsExecute() noexcept {
		kengine::registerComponents<kengine::functions::Execute>();

	}
}