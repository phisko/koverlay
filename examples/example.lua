TOOL_NAME = "Lua"

shouldDraw, TOOL_ENABLED = imgui.Begin("Lua", TOOL_ENABLED)
if shouldDraw then
	imgui.BeginChild("child", 0, 50 * IMGUI_SCALE, true)
	imgui.EndChild()
end
imgui.End()
