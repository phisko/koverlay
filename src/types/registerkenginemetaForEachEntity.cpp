#include "helpers/registerTypeHelper.hpp"
#include "meta/ForEachEntity.hpp"

namespace types{
	void registerkenginemetaForEachEntity() noexcept {
		kengine::registerComponents<kengine::meta::ForEachEntity>();

	}
}