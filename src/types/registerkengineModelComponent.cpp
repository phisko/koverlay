#include "helpers/registerTypeHelper.hpp"
#include "data/ModelComponent.hpp"

namespace types{
	void registerkengineModelComponent() noexcept {
		kengine::registerComponents<kengine::ModelComponent>();

	}
}