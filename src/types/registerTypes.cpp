
#include "helpers/logHelper.hpp"

namespace types{
	void registerTypes() noexcept {
		kengine_log(Log, "Init", "Registering types");

		extern void registerkengineAdjustableComponent() noexcept;
		registerkengineAdjustableComponent();

		extern void registerkengineWindowComponent() noexcept;
		registerkengineWindowComponent();

		extern void registerkengineCommandLineComponent() noexcept;
		registerkengineCommandLineComponent();

		extern void registerkenginefunctionsExecute() noexcept;
		registerkenginefunctionsExecute();

		extern void registerkengineImGuiToolComponent() noexcept;
		registerkengineImGuiToolComponent();

		extern void registerkengineInstanceComponent() noexcept;
		registerkengineInstanceComponent();

		extern void registerkengineLuaComponent() noexcept;
		registerkengineLuaComponent();

		extern void registerkengineLuaTableComponent() noexcept;
		registerkengineLuaTableComponent();

		extern void registerkengineModelComponent() noexcept;
		registerkengineModelComponent();

		extern void registerkengineNameComponent() noexcept;
		registerkengineNameComponent();

		extern void registerkenginePythonComponent() noexcept;
		registerkenginePythonComponent();

		extern void registerkengineSelectedComponent() noexcept;
		registerkengineSelectedComponent();

		extern void registerkengineTimeModulatorComponent() noexcept;
		registerkengineTimeModulatorComponent();

		extern void registerkenginemetaAttachTo() noexcept;
		registerkenginemetaAttachTo();

		extern void registerkenginemetaAttributes() noexcept;
		registerkenginemetaAttributes();

		extern void registerkenginemetaCopy() noexcept;
		registerkenginemetaCopy();

		extern void registerkenginemetaCount() noexcept;
		registerkenginemetaCount();

		extern void registerkenginemetaDetachFrom() noexcept;
		registerkenginemetaDetachFrom();

		extern void registerkenginemetaDisplayImGui() noexcept;
		registerkenginemetaDisplayImGui();

		extern void registerkenginemetaEditImGui() noexcept;
		registerkenginemetaEditImGui();

		extern void registerkenginemetaForEachEntity() noexcept;
		registerkenginemetaForEachEntity();

		extern void registerkenginemetaForEachEntityWithout() noexcept;
		registerkenginemetaForEachEntityWithout();

		extern void registerkenginemetaGet() noexcept;
		registerkenginemetaGet();

		extern void registerkenginemetaHas() noexcept;
		registerkenginemetaHas();

		extern void registerkenginemetaLoadFromJSON() noexcept;
		registerkenginemetaLoadFromJSON();

		extern void registerkenginemetaMatchString() noexcept;
		registerkenginemetaMatchString();

		extern void registerkenginemetaSaveToJSON() noexcept;
		registerkenginemetaSaveToJSON();

		extern void registerkenginemetaSize() noexcept;
		registerkenginemetaSize();

	}
}
