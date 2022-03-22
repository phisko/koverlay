#include "ImGuiLuaSystem.hpp"
#include "kengine.hpp"

// kengine data
#include "data/CommandLineComponent.hpp"
#include "data/ImGuiScaleComponent.hpp"
#include "data/ImGuiToolComponent.hpp"
#include "data/LuaStateComponent.hpp"
#include "data/NameComponent.hpp"

// kengine functions
#include "functions/Execute.hpp"

// putils
#include "command_line_arguments.hpp"
#include "Directory.hpp"

namespace {
    struct Options {
        bool printLuaFunctions = false;
    };
}

#define refltype Options
putils_reflection_info{
    putils_reflection_custom_class_name(ImGui Lua);
    putils_reflection_attributes(
        putils_reflection_attribute(printLuaFunctions,
            putils_reflection_metadata("help", "Print a list of all ImGui functions exposed to Lua")
        )
    );
};
#undef refltype

extern void LoadImguiBindings();
extern lua_State * lState;

namespace {
    struct impl {
        static inline sol::state * g_state = nullptr;

        static void init(kengine::Entity &system) noexcept {
            initBindings();
            system += kengine::functions::Execute{[&](float deltaTime) noexcept {
                runScripts();
            }};
        }

        static void initBindings() noexcept {
            Options options;
            for (const auto & [e, args] : kengine::entities.with<kengine::CommandLineComponent>())
                options = putils::parseArguments<Options>(args.arguments);

            for (const auto &[e, state]: kengine::entities.with<kengine::LuaStateComponent>()) {
                lState = *state.state;
                LoadImguiBindings();
                g_state = state.state;
                if (options.printLuaFunctions)
                    g_state->script(
                        R"(
print("ImGui Lua bindings:")
for k, v in pairs(imgui) do
    print("\t", k)
end
)");
            }
        }

        static void runScripts() noexcept {
            (*g_state)["IMGUI_SCALE"] = getScale();

            putils::Directory d("scripts");

            d.for_each([&](const putils::Directory::File &f) {
                const auto view = std::string_view(f.name);
                const auto dot = view.find_last_of('.');
                if (!f.isDirectory && dot != std::string_view::npos && view.substr(dot) == ".lua")
                    runScript(f.fullPath.c_str());
            });
        }

        static float getScale() noexcept {
            float scale = 1.f;
            for (const auto &[e, comp]: kengine::entities.with<kengine::ImGuiScaleComponent>())
                scale *= comp.scale;
            return scale;
        }

        static void runScript(const char *script) noexcept {
            auto e = getEntityForScript(script);
            auto &tool = e.get<kengine::ImGuiToolComponent>();

            if (!tool.enabled)
                return;

            (*g_state)["TOOL_ENABLED"] = tool.enabled;
            try {
                g_state->script_file(script);
            }
            catch (const std::exception &e) {
                std::cerr << e.what() << std::endl;
            }
            tool.enabled = (*g_state)["TOOL_ENABLED"];
        }

        static kengine::Entity getEntityForScript(const char *script) noexcept {
            static std::unordered_map<std::string, kengine::EntityID> ids;

            const auto it = ids.find(script);
            if (it != ids.end())
                return kengine::entities[it->second];

            return kengine::entities.create([&](kengine::Entity &e) {
                ids[script] = e.id;
                try {
                    g_state->script_file(script);
                }
                catch (const std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
                e += kengine::ImGuiToolComponent{(*g_state)["TOOL_ENABLED"]};
                const std::string name = (*g_state)["TOOL_NAME"];
                e += kengine::NameComponent{name};
            });
        }
    };
}

kengine::EntityCreator * ImGuiLuaSystem() noexcept {
	return impl::init;
}