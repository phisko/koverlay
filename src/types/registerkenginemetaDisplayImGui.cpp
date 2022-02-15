#include "helpers/registerTypeHelper.hpp"
#include "meta/DisplayImGui.hpp"

namespace types{
	void registerkenginemetaDisplayImGui() noexcept {
		kengine::registerComponents<kengine::meta::DisplayImGui>();

	}
}