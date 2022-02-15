#include "helpers/registerTypeHelper.hpp"
#include "data/WindowComponent.hpp"

namespace types{
	void registerkengineWindowComponent() noexcept {
		kengine::registerComponents<kengine::WindowComponent>();

	}
}