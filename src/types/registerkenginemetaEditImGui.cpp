#include "helpers/registerTypeHelper.hpp"
#include "meta/EditImGui.hpp"

namespace types{
	void registerkenginemetaEditImGui() noexcept {
		kengine::registerComponents<kengine::meta::EditImGui>();

	}
}