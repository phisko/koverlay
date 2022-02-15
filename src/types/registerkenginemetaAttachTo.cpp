#include "helpers/registerTypeHelper.hpp"
#include "meta/AttachTo.hpp"

namespace types{
	void registerkenginemetaAttachTo() noexcept {
		kengine::registerComponents<kengine::meta::AttachTo>();

	}
}