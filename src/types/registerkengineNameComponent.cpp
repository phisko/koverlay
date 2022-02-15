#include "helpers/registerTypeHelper.hpp"
#include "data/NameComponent.hpp"

namespace types{
	void registerkengineNameComponent() noexcept {
		kengine::registerComponents<kengine::NameComponent>();

	}
}