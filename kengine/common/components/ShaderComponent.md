# [ShaderComponent](ShaderComponent.hpp)

`Components` that let `Entities` be used as custom shaders.

Shaders can either be:
* GBuffer shaders: used to fill the deferred rendering GBuffer
* Lighting shaders: reading the GBuffer and applying lighting to fill the screen
* Post-process shaders

### Helper functions

These functions take as a template parameter the shader [Program](../../putils/opengl/Program.md) type.

```cpp
template<typename T, typename ...Args>
GBufferShaderComponent makeGBufferShaderComponent(Args && ... args); 

template<typename T, typename ...Args>
LightingShaderComponent makeLightingShaderComponent(Args && ... args);

template<typename T, typename ...Args>
PostProcessShaderComponent makePostProcessShaderComponent(Args && ... args);
```

### Common base

##### shader

```cpp
std::unique_ptr<putils::gl::Program> shader;
```

Pointer to the shader [Program](../../putils/opengl/Program.md).

##### enabled

```cpp
bool enabled;
```

# [ShadowMapShaderComponent](ShaderComponent.hpp)

Used to tag an `Entity` whose `ShaderComponent` actually points to a `ShadowMapShader` (which should be processed to fill "light entities'" shadow maps).

# [ShadowCubeShaderComponent](ShaderComponent.hpp)

Used to tag an `Entity` whose `ShaderComponent` actually points to a `ShadowCubeShader` (which should be processed to fill "light entities'" shadow cubes).