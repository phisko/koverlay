#include "helpers/registerTypeHelper.hpp"
#include "data/ImGuiToolComponent.hpp"

namespace types{
	void registerkengineImGuiToolComponent() noexcept {
		kengine::registerComponents<kengine::ImGuiToolComponent>();

	}
}