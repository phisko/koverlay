#include "helpers/registerTypeHelper.hpp"
#include "meta/SaveToJSON.hpp"

namespace types{
	void registerkenginemetaSaveToJSON() noexcept {
		kengine::registerComponents<kengine::meta::SaveToJSON>();

	}
}