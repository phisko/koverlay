#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#endif

#include "HTTPRequest.hpp"
#include "ClosestPlaneSystem.hpp"

#include "EntityManager.hpp"
#include "Export.hpp"

#include "packets/AddImGuiTool.hpp"
#include "components/ImGuiComponent.hpp"
#include "json.hpp"
#include "to_string.hpp"
#include "imgui.h"


EXPORT kengine::ISystem * getSystem(kengine::EntityManager & em) {
	return new ClosestPlaneSystem(em);
}

static std::string text;

ClosestPlaneSystem::ClosestPlaneSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "Closest plane", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&em] {
				if (!display)
					return;

				if (ImGui::Begin("Closest plane")) {
					ImGui::Text(text.c_str());
				}
				ImGui::End();
		});
	};

	http::Request request("http://opensky-network.org/api/states/all");
	const auto response = request.send("POST", {
		{ "lamin", "45.8389" },
		{ "lomin", "5.9962" },
		{ "lamax", "47.8229" },
		{ "lomax", "10.5226" }
	});
	text = std::string(response.body.begin(), response.body.end());
}