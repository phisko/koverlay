#include <mutex>
#include <ctime>
#include <filesystem>

#include "ClosestPlaneSystem.hpp"

#include "EntityManager.hpp"
#include "Export.hpp"

#include "packets/AddImGuiTool.hpp"
#include "components/ImGuiComponent.hpp"
#include "json.hpp"
#include "to_string.hpp"
#include "imgui.h"

#include "csv.hpp"
#include "curl.hpp"
#include "python.hpp"

static float * g_scale = nullptr;
static float getScale() {
	return g_scale != nullptr ? *g_scale : 1.f;
}

EXPORT kengine::ISystem * getSystem(kengine::EntityManager & em) {
	return new ClosestPlaneSystem(em);
}

struct PlaneInfo {
	std::string callSign;
	std::string country;

	std::string company;
	std::string flightNumber;

	std::string origin;
	std::string destination;
};

struct AirportInfo {
	std::string airportName;
	std::string city;
	std::string country;
	std::string code;
};

struct Coordinates {
	float latitudeA = 48.801490f;
	float latitudeB = 48.701494f;
	float longitudeA = 2.338643f;
	float longitudeB = 2.713339f;
};
static Coordinates g_coordinates;
static std::string g_filterAirport;

static std::unordered_map<std::string, AirportInfo> g_airports;
static std::unordered_map<std::string, std::string> g_airlines;

static std::thread g_thread;
static std::string g_lastResponse;
static std::vector<PlaneInfo> g_planes;
static std::vector<PlaneInfo> g_pendingPlanes;
static std::mutex g_planesMutex;
static float g_planesUpdateTimer = 0.f;
static float g_planesUpdateFrequency = 5.f;

static void getPendingPlaneList(float deltaTime) {
	std::unique_lock<std::mutex> l(g_planesMutex);
	bool first = true;
	if (g_planes.empty()) {
		if (!g_pendingPlanes.empty()) {
			g_planes = g_pendingPlanes;
			g_pendingPlanes.clear();
			g_planesUpdateTimer = g_planesUpdateFrequency;
		}
		else
			ImGui::Text(g_lastResponse.c_str());
	}

	g_planesUpdateTimer -= deltaTime;
	if (g_planesUpdateTimer < 0.f && !g_pendingPlanes.empty()) {
		g_planes = g_pendingPlanes;
		g_pendingPlanes.clear();
		g_planesUpdateTimer = g_planesUpdateFrequency;
	}
}

ClosestPlaneSystem::ClosestPlaneSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "Closest plane", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
				if (!display)
					return;

				if (ImGui::Begin("Closest plane", &display)) {
					const auto inputFloat = [](const char * label, float & f) {
						float tmp = f;
						if (ImGui::InputFloat(label, &tmp, 0.f, 0.f, "%.6f"))
							f = tmp;
					};

					char buff[1024];
					strcpy_s(buff, g_filterAirport.c_str());
					if (ImGui::InputText("Airport", buff, sizeof(buff)))
						g_filterAirport = buff;
					ImGui::InputFloat("Update frequency", &g_planesUpdateFrequency);

					ImGui::Text("Latitudes");
					ImGui::Columns(2);
					inputFloat("##latitudeA", g_coordinates.latitudeA);
					ImGui::NextColumn();
					inputFloat("##latitudeB", g_coordinates.latitudeB);
					ImGui::Columns();

					ImGui::Text("Longitudes");
					ImGui::Columns(2);
					inputFloat("##longitudeA", g_coordinates.longitudeA);
					ImGui::NextColumn();
					inputFloat("##longitudeB", g_coordinates.longitudeB);
					ImGui::Columns();

					if (g_coordinates.latitudeA > g_coordinates.latitudeB)
						std::swap(g_coordinates.latitudeA, g_coordinates.latitudeB);
					if (g_coordinates.longitudeA > g_coordinates.longitudeB)
						std::swap(g_coordinates.longitudeA, g_coordinates.longitudeB);

					getPendingPlaneList(time.getDeltaTime().count());

					for (const auto & plane : g_planes) {
						const auto & origin = g_airports[plane.origin].airportName;
						const auto & destination = g_airports[plane.destination].airportName;
						if (!g_filterAirport.empty() && origin.find(g_filterAirport) == std::string::npos && destination.find(g_filterAirport) == std::string::npos)
							continue;

						ImGui::Separator();
						ImGui::Separator();

						static const auto columns = [](const std::string & l, const std::string & r) {
							ImGui::Text(l.c_str());
							ImGui::NextColumn();
							ImGui::Text(r.c_str());
							ImGui::NextColumn();
						};

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 100.f * getScale());

						columns("Callsign", plane.callSign);
						columns("Country", plane.country);
						columns("Airline", g_airlines[plane.company]);
						columns("Flight", plane.company + plane.flightNumber);

						static const auto displayAirport = [&](const std::string & childName, const AirportInfo & airport) {
							ImGui::BeginChild(childName.c_str(), { 0, 80 * getScale() }, true);
							ImGui::Columns(2);
							ImGui::SetColumnWidth(0, 75.f * getScale());
							columns("Name", airport.airportName);
							columns("City", airport.city);
							columns("Country", airport.country);
							columns("Code", airport.code);
							ImGui::Columns();
							ImGui::EndChild();
						};

						ImGui::Text("Origin");
						ImGui::NextColumn();
						displayAirport("Origin##" + plane.callSign, g_airports[plane.origin]);
						ImGui::NextColumn();
						ImGui::Text("Destination");
						ImGui::NextColumn();
						displayAirport("Destination##" + plane.callSign, g_airports[plane.destination]);
						ImGui::NextColumn();

						ImGui::Columns();
					}
				}
				ImGui::End();
		});
	};
}

ClosestPlaneSystem::~ClosestPlaneSystem() {
	g_thread.join();
}

static constexpr auto g_airportsFile = "closestPlane/airports.dat";

static void downloadAirports() {
	putils::curl::downloadFile("https://raw.githubusercontent.com/jpatokal/openflights/master/data/airports.dat", g_airportsFile);
	const auto lastWriteTime = std::filesystem::last_write_time(g_airportsFile);
	const auto now = decltype(lastWriteTime)::clock().now();
	assert(now - lastWriteTime < std::chrono::seconds(60));
}

static bool shouldDownloadAirports() {
	if (!std::filesystem::exists(g_airportsFile))
		return true;

	const auto lastWriteTime = std::filesystem::last_write_time(g_airportsFile);
	const auto now = decltype(lastWriteTime)::clock().now();
	if (now - lastWriteTime > std::chrono::hours(24))
		return true;

	return false;
}

static void parseAirports() {
	std::ifstream f(g_airportsFile);
	assert(f);
	for (std::string line; std::getline(f, line);) {
		const auto fields = putils::parseCSVLine(line, ',');
		AirportInfo airport;
		airport.airportName = fields[1];
		airport.city = fields[2];
		airport.country = fields[3];
		airport.code = fields[4];

		const auto & icao = fields[5];
		g_airports[icao] = std::move(airport);
	}
}

static void parseAirlines() {
	size_t count = 0;

	std::ifstream f("closestPlane/airlines.dat");
	assert(f);
	for (std::string line; std::getline(f, line);) {
		++count;
		const auto fields = putils::parseCSVLine(line, '|');
		const auto & iataCode = fields[0];
		const auto & name = fields[2];
		g_airlines[iataCode] = name;
	}
}

static PlaneInfo getPlaneInfo(const std::string & callSign, const std::string & country) {
	PlaneInfo plane;

	plane.callSign = callSign;
	plane.country = country;

	const auto t = ::time(nullptr);
	const auto s = putils::curl::httpRequest("https://opensky-network.org/api/routes", {
		{ "callsign", plane.callSign }
		});

	if (!putils::json::accept(s))
		return plane;

	const auto json = putils::json::parse(s);

	plane.company = json["operatorIata"].get<std::string>();
	plane.flightNumber = putils::toString(json["flightNumber"].get<int>());

	plane.origin = json["route"][0].get<std::string>();
	plane.destination = json["route"][1].get<std::string>();

	return plane;
}

void ClosestPlaneSystem::execute() {
	static bool first = true;
	if (!first)
		return;
	first = false;

	g_thread = std::thread([&] {
		if (shouldDownloadAirports())
			downloadAirports();

		parseAirports();
		parseAirlines();

		while (_em.running) {
			const auto s = putils::curl::httpRequest("https://opensky-network.org/api/states/all", {
				{ "lamin", putils::toString(g_coordinates.latitudeA) },
				{ "lomin", putils::toString(g_coordinates.longitudeA) },
				{ "lamax", putils::toString(g_coordinates.latitudeB) },
				{ "lomax", putils::toString(g_coordinates.longitudeB) }
				});

			if (!_em.running)
				break;

			g_lastResponse = s;
			if (!putils::json::accept(s))
				continue;

			const auto json = putils::json::parse(s);
			std::vector<PlaneInfo> planes;

			for (const auto & state : json["states"]) {
				planes.push_back(getPlaneInfo(state[1].get<std::string>(), state[2].get<std::string>()));
				if (!_em.running)
					break;

				std::unique_lock<std::mutex> l(g_planesMutex);
				g_pendingPlanes = planes;
			}
		}
	});
}

void ClosestPlaneSystem::handle(const kengine::packets::ImGuiScale & p) const {
	g_scale = &p.scale;
}