#include "helpers/registerTypeHelper.hpp"
#include "meta/Get.hpp"

namespace types{
	void registerkenginemetaGet() noexcept {
		kengine::registerComponents<kengine::meta::Get>();

	}
}