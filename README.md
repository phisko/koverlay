Simple and extensible overlay system using ImGui and the [Kengine](https://github.com/phisko/kengine).

ImGui windows are rendered as "always-on-top" windows over other Windows applications.

![koverlay](https://github.com/phisko/koverlay/blob/master/screenshot.png)

# Bar
A top-screen menubar lets you start/stop various tools. This bar is automatically hidden when the overlay loses focus. Double clicking on the system tray icon will bring it back in focus.

# Keyboard shortcuts
Alt + W: grab focus and display the top-screen menubar
Alt + Q: enable/disable the overlay

# Creating tools

Tools can be created in three different ways: lua scripts, C++ plugins, and `kengine` Systems (which are also loaded as plugins). All these tools may be dropped in and hot-loaded at runtime, although modification and re-loading is not supported.

## Lua scripts

Scripts can be added to the `scripts` directory, next to the executable, and will be automatically loaded.

Lua scripts use the [ImGui lua bindings](https://github.com/patrickriordan/imgui_lua_bindings).

Scripts should define a global `TOOL_NAME` variable. This will be used by the overlay to provide an entry for the tool in the top-screen menubar, as well as system tray icon's context menu.

Scripts should also set a global `TOOL_ENABLED` variable according to what `imgui.Begin()` returns as its second parameter, e.g.:

```lua
shouldDraw, TOOL_ENABLED = imgui.Begin("Example", TOOL_ENABLED)
```

Scripts may also access the `IMGUI_SCALE` global variable. This is a floating point value that should be used to scale child boxes and such. Users may modify this freely to adapt the overlay to their screen DPI. Note that texts are automatically scaled.

### Example

An example lua script can be found [here](examples/example.lua).

## C++ plugins

Plugins can be added to the `plugins` directory, next to the executable, and will be automatically loaded.

Plugins should link with the `ImGui` version provided in `examples/newPlugin` to ensure ABI compatibility (as the internal `ImGui` data structures may change between versions).

Plugins should include the [framework.hpp](examples/newPlugin/framework.hpp) file provided in `examples/newPlugin`. For those who care, this defines some trampoline functions which take care of getting the `GImGui` context from the main executable's address space and setting up a `PLUGIN_ENABLED` variable, used to identify the state of the tool for the system tray context menu and the top-screen menubar.

Plugins simply have to define a `const char * getName()` function and a `void imguiFunction(float scale)` function. The `scale` parameter is a floating point value that should be used to scale child boxes and such. Users may modify this freely to adapt the overlay to their screen DPI. Note that texts are automatically scaled.

### Example

An example plugin can be found [here](examples/newPlugin/NewPlugin.cpp).

## Kengine plugins

As the overlay uses the [Kengine](https://github.com/phisko/kengine), it can load plugins which provide systems for the engine (which may do anything you want them to, and access each other's entities and components to share data between systems).

These plugins can be added to the `plugins` directory, next to the executable, and will be automatically loaded.

Plugins should link with the `kengine` version used to compile the overlay's main executable to ensure ABI compatibility (as the internal `kengine` data structures may change between versions).

Plugins simply have to define a `kengine::ISystem * getSystem(kengine::EntityManager & em)` function that returns a pointer to the user-defined system.

The custom system should handle the [ImGuiScale](kengine/common/packets/ImGuiScale.hpp) datapacket, which provides a reference to a `scale` variable. This is a floating point value that should be used to scale child boxes and such. Users may modify this freely to adapt the overlay to their screen DPI. Note that texts are automatically scaled.

### Example

An example system can be found [here](examples/newSystem/NewSystem.cpp).