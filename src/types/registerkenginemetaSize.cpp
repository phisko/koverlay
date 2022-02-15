#include "helpers/registerTypeHelper.hpp"
#include "meta/Size.hpp"

namespace types{
	void registerkenginemetaSize() noexcept {
		kengine::registerComponents<kengine::meta::Size>();

	}
}