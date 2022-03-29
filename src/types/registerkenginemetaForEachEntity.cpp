#include "helpers/registerTypeHelper.hpp"
#include "meta/ForEachEntity.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkenginemetaForEachEntity() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::meta::ForEachEntity'");
		kengine::registerComponents<kengine::meta::ForEachEntity>();

	}
}