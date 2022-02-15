#include "helpers/registerTypeHelper.hpp"
#include "meta/LoadFromJSON.hpp"

namespace types{
	void registerkenginemetaLoadFromJSON() noexcept {
		kengine::registerComponents<kengine::meta::LoadFromJSON>();

	}
}