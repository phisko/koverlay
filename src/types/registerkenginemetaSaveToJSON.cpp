#include "helpers/registerTypeHelper.hpp"
#include "meta/SaveToJSON.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkenginemetaSaveToJSON() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::meta::SaveToJSON'");
		kengine::registerComponents<kengine::meta::SaveToJSON>();

	}
}