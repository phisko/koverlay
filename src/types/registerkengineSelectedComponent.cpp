#include "helpers/registerTypeHelper.hpp"
#include "data/SelectedComponent.hpp"

namespace types{
	void registerkengineSelectedComponent() noexcept {
		kengine::registerComponents<kengine::SelectedComponent>();

	}
}