#include <mutex>
#include <ctime>

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

struct PlaneInfo {
	std::string callSign;
	std::string country;
	std::string departureTime;
	std::string origin;
	std::string destination;
};

static std::thread g_thread;

static std::vector<PlaneInfo> g_planes;
std::mutex g_planesMutex;

ClosestPlaneSystem::ClosestPlaneSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "Closest plane", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
				if (!display)
					return;

				if (ImGui::Begin("Closest plane")) {
					std::unique_lock<std::mutex> l(g_planesMutex);
					for (const auto & plane : g_planes) {
						if (ImGui::CollapsingHeader(plane.callSign.c_str())) {
							ImGui::Columns(2);
							ImGui::Text("Country"); ImGui::NextColumn(); ImGui::Text(plane.country.c_str()); ImGui::NextColumn();
							ImGui::Text("Origin"); ImGui::NextColumn(); ImGui::Text(plane.origin.c_str()); ImGui::NextColumn();
							ImGui::Text("Destination"); ImGui::NextColumn(); ImGui::Text(plane.destination.c_str()); ImGui::NextColumn();
							ImGui::Columns();
						}
					}
					// for (const auto & state : g_json) {
					// 	const auto callSign = state[1].dump();
					// 	if (ImGui::CollapsingHeader(callSign.c_str())) {
					// 		ImGui::Columns(2);
					// 		ImGui::Text("Origin country"); ImGui::NextColumn(); ImGui::Text(state[2].dump().c_str());
					// 		ImGui::Text("Origin country"); ImGui::NextColumn(); ImGui::Text(state[2].dump().c_str());
					// 		ImGui::Columns();
					// 	}
					// }
				}
				ImGui::End();
		});
	};
}

static std::string runProcess(const std::string & process) {
	std::string s;

	const auto pipe = _popen(process.c_str(), "r");
	assert(pipe != nullptr);
	char buffer[1024];
	while (fgets(buffer, sizeof(buffer), pipe))
		s += buffer;
	_pclose(pipe);

	return s;
}

void ClosestPlaneSystem::execute() {
	static bool first = true;
	if (!first)
		return;
	first = false;

	g_thread = std::thread([&] {
		try {
			while (_em.running) {
				// py::eval_file("closestPlane/get_info.py");
				// g_json = putils::json(py::str(py::globals()["closestPlane"]));

				const auto s = runProcess("curl -s \"https://opensky-network.org/api/states/all?lamin=48.724017&lomin=2.356484&lamax=48.775232&lomax=2.539622\"");
				const auto json = putils::json::parse(s);

				std::vector<PlaneInfo> planes;

				for (const auto & state : json["states"]) {
					PlaneInfo plane;
					plane.callSign = state[1].dump();
					plane.country = state[2].dump();

					const auto t = ::time(nullptr);
					const auto s = runProcess("curl -s \"https://opensky-network.org/api/flights/aircraft?icao24=" + state[0].dump() + "&begin=" + putils::toString(t - 86400) + "&end=" + putils::toString(t) + "\"");
					const auto json = putils::json::parse(s);

					for (const auto & flight : json) {
						plane.origin = flight["estDepartureAirport"].dump();
						plane.destination = flight["estArrivalAirport"].dump();
					}

					planes.push_back(std::move(plane));
				}

				std::unique_lock<std::mutex> l(g_planesMutex);
				g_planes = planes;
			}
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << '\n';
		}
	});
}