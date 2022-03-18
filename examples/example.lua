TOOL_NAME = "Lua"
-- can use IMGUI_SCALE to properly scale child windows

if EXAMPLE_BUTTON_CLICKED == nil then
    EXAMPLE_BUTTON_CLICKED = false
end

shouldDraw, TOOL_ENABLED = imgui.Begin("Lua", TOOL_ENABLED)
if shouldDraw then
    if imgui.SmallButton("Click me") then
        EXAMPLE_BUTTON_CLICKED = true
    end

    if EXAMPLE_BUTTON_CLICKED then
        imgui.TextUnformatted("Good job!")
    end
end
imgui.End()
