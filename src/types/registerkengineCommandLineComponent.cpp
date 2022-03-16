#include "helpers/registerTypeHelper.hpp"
#include "data/CommandLineComponent.hpp"

namespace types{
	void registerkengineCommandLineComponent() noexcept {
		kengine::registerComponents<kengine::CommandLineComponent>();

	}
}