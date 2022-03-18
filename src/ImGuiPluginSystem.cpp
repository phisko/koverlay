#include "ImGuiPluginSystem.hpp"
#include "PluginManager.hpp"
#include "kengine.hpp"

// kengine data
#include "data/NameComponent.hpp"
#include "data/ImGuiScaleComponent.hpp"
#include "data/ImGuiToolComponent.hpp"

// kengine functions
#include "functions/Execute.hpp"


struct ImGuiContext;
extern ImGuiContext * GImGui;

namespace {
    struct impl {
        static inline putils::PluginManager pm;

        static void init(kengine::Entity &system) noexcept {
            system += kengine::functions::Execute{execute};
        }

        struct PluginEnabledPointerComponent {
            bool *enabled;
        };

        static void execute(float deltaTime) noexcept {
            bool *enabled;
            const auto names = pm.rescanDirectoryWithReturn<1, const char *>("plugins", "getNameAndEnabled", &enabled);
            for (const auto &name: names) {
                kengine::entities += [&](kengine::Entity &e) {
                    e += kengine::NameComponent{name};
                    e += PluginEnabledPointerComponent{enabled};
                    e += kengine::ImGuiToolComponent{*enabled};
                };
            }

            drawImGui();
        }

        static void drawImGui() noexcept {
            for (const auto &[e, enabled, tool]: kengine::entities.with<PluginEnabledPointerComponent, kengine::ImGuiToolComponent>())
                *enabled.enabled = tool.enabled;

            pm.execute("drawImGui", *GImGui, getScale());

            for (const auto &[e, enabled, tool]: kengine::entities.with<PluginEnabledPointerComponent, kengine::ImGuiToolComponent>())
                tool.enabled = *enabled.enabled;
        }

        static float getScale() noexcept {
            float scale = 1.f;
            for (const auto &[e, comp]: kengine::entities.with<kengine::ImGuiScaleComponent>())
                scale *= comp.scale;
            return scale;
        }
    };
}

kengine::EntityCreator * ImGuiPluginSystem() noexcept {
	return impl::init;
}