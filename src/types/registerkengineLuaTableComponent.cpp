#include "helpers/registerTypeHelper.hpp"
#include "data/LuaTableComponent.hpp"

namespace types{
	void registerkengineLuaTableComponent() noexcept {
		kengine::registerComponents<kengine::LuaTableComponent>();

	}
}