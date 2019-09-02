#include "ClosestPlaneSystem.hpp"

#include "EntityManager.hpp"
#include "Export.hpp"

#include "packets/AddImGuiTool.hpp"
#include "components/ImGuiComponent.hpp"
#include "json.hpp"
#include "to_string.hpp"
#include "imgui.h"

#include "python.hpp"

EXPORT kengine::ISystem * getSystem(kengine::EntityManager & em) {
	return new ClosestPlaneSystem(em);
}

static std::thread g_thread;
static std::string g_text;

ClosestPlaneSystem::ClosestPlaneSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "Closest plane", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
				if (!display)
					return;

				if (ImGui::Begin("Closest plane")) {
					ImGui::Text(g_text.c_str());
				}
				ImGui::End();
		});
	};
}

void ClosestPlaneSystem::execute() {
	static bool first = true;
	if (!first)
		return;
	first = false;

	g_thread = std::thread([&] {
		try {
			while (_em.running) {
				py::eval_file("closestPlane/get_info.py");
				g_text = py::str(py::globals()["closestPlane"]);
			}
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << '\n';
		}
	});
}