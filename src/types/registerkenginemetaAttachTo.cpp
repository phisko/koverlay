#include "helpers/registerTypeHelper.hpp"
#include "meta/AttachTo.hpp"
#include "helpers/logHelper.hpp"

namespace types{
	void registerkenginemetaAttachTo() noexcept {
		kengine_log(Log, "Init/registerTypes", "Registering 'kengine::meta::AttachTo'");
		kengine::registerComponents<kengine::meta::AttachTo>();

	}
}