#include "helpers/registerTypeHelper.hpp"
#include "meta/Attributes.hpp"

namespace types{
	void registerkenginemetaAttributes() noexcept {
		kengine::registerComponents<kengine::meta::Attributes>();

	}
}