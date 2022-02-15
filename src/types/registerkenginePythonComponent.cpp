#include "helpers/registerTypeHelper.hpp"
#include "data/PythonComponent.hpp"

namespace types{
	void registerkenginePythonComponent() noexcept {
		kengine::registerComponents<kengine::PythonComponent>();

	}
}