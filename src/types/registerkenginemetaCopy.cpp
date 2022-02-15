#include "helpers/registerTypeHelper.hpp"
#include "meta/Copy.hpp"

namespace types{
	void registerkenginemetaCopy() noexcept {
		kengine::registerComponents<kengine::meta::Copy>();

	}
}