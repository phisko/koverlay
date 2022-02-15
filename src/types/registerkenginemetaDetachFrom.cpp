#include "helpers/registerTypeHelper.hpp"
#include "meta/DetachFrom.hpp"

namespace types{
	void registerkenginemetaDetachFrom() noexcept {
		kengine::registerComponents<kengine::meta::DetachFrom>();

	}
}