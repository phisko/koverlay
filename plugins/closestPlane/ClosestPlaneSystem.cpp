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

	std::string company;
	std::string flightNumber;

	std::string origin;
	std::string destination;

	putils::json json;
};

struct AirportInfo {
	std::string airportName;
	std::string city;
	std::string country;
	std::string code;
};

static std::thread g_thread;

static std::string g_lastResponse;
static std::vector<PlaneInfo> g_planes;
static std::unordered_map<std::string, AirportInfo> g_airports;
std::mutex g_planesMutex;

static void displayJSON(const char * name, const putils::json & json) {
	if (json.is_string())
		ImGui::Text("%s: %s", name, json.get<std::string>().c_str());
	else if (json.is_null())
		ImGui::Text("%s: null", name);
	else if (json.is_boolean())
		ImGui::Text("%s: %s", name, json.get<bool>() ? "true" : "false");
	else if (json.is_number_float())
		ImGui::Text("%s: %f", name, json.get<float>());
	else if (json.is_number_integer())
		ImGui::Text("%s: %d", name, json.get<int>());
	else if (json.is_number_unsigned())
		ImGui::Text("%s: %zu", name, json.get<unsigned int>());
	else if (json.is_object()) {
		if (ImGui::TreeNode(name)) {
			for (const auto & obj : json.items())
				displayJSON(obj.key().c_str(), obj.value());
			ImGui::TreePop();
		}
	}
	else if (json.is_array()) {
		if (ImGui::TreeNode(name)) {
			size_t i = 0;
			for (const auto & obj : json)
				displayJSON(putils::toString(i++).c_str(), obj);
			ImGui::TreePop();
		}
	}
}

ClosestPlaneSystem::ClosestPlaneSystem(kengine::EntityManager & em) : System(em), _em(em) {
	static bool display = false;
	send(kengine::packets::AddImGuiTool{ "Closest plane", display });

	em += [&](kengine::Entity & e) {
		e += kengine::ImGuiComponent([&] {
				if (!display)
					return;

				if (ImGui::Begin("Closest plane")) {
					std::unique_lock<std::mutex> l(g_planesMutex);
					bool first = true;
					if (g_planes.empty())
						ImGui::Text(g_lastResponse.c_str());

					for (const auto & plane : g_planes) {
						if (!first)
							ImGui::Separator();
						first = false;

						static const auto columns = [](const std::string & l, const std::string & r) {
							ImGui::Text(l.c_str());
							ImGui::NextColumn();
							ImGui::Text(r.c_str());
							ImGui::NextColumn();
						};

						ImGui::Columns(2);
						ImGui::SetColumnWidth(0, 100.f);

						columns("Callsign", plane.callSign);
						columns("Country", plane.country);
						columns("Flight", plane.company + plane.flightNumber);

						static const auto displayAirport = [&](const std::string & childName, const AirportInfo & airport) {
							ImGui::BeginChild(childName.c_str(), { 0, 80 }, true);
							ImGui::Columns(2);
							ImGui::SetColumnWidth(0, 75.f);
							ImGui::Text("Name"); ImGui::NextColumn(); ImGui::Text(airport.airportName.c_str()); ImGui::NextColumn();
							ImGui::Text("City"); ImGui::NextColumn(); ImGui::Text(airport.city.c_str()); ImGui::NextColumn();
							ImGui::Text("Country"); ImGui::NextColumn(); ImGui::Text(airport.country.c_str()); ImGui::NextColumn();
							ImGui::Text("Code"); ImGui::NextColumn(); ImGui::Text(airport.code.c_str()); ImGui::NextColumn();
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

						displayJSON("json", plane.json);
					}
				}
				ImGui::End();
		});
	};
}

ClosestPlaneSystem::~ClosestPlaneSystem() {
	g_thread.join();
}

static std::string makeCurlCommand(const std::string & base, const std::unordered_map<std::string, std::string> & params = {}) {
	std::string ret = base;

	if (!params.empty())
		ret += '?';

	bool first = true;
	for (const auto &[k, v] : params) {
		if (!first)
			ret += '&';
		first = false;

		ret += k;
		ret += '=';
		ret += v;
	}

	return "curl -s \"" + ret + '"';
}


static constexpr auto g_airportsFile = "closestPlane/airports.dat";

static void downloadAirports() {
	system((makeCurlCommand("https://raw.githubusercontent.com/jpatokal/openflights/master/data/airports.dat") + " > " + g_airportsFile).c_str());
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

std::vector<std::string> parseCSVLine(const std::string & s, char delim) {
	static size_t lines = 0;

	++lines;
	std::vector<std::string> ret;

	size_t startIndex = 0;
	bool stop = false;
	while (!stop) {
		size_t nextStartIndex;

		size_t endIndex;
		if (s[startIndex] == '"') {
			++startIndex; // Pop opening quote
			const auto closingQuote = s.find_first_of('"', startIndex);
			endIndex = s.find_first_of(delim, closingQuote);
			if (endIndex == std::string::npos) {
				endIndex = s.size() - 1;
				stop = true;
			}
			nextStartIndex = endIndex + 1;
			--endIndex; // Pop closing quote
		}
		else {
			endIndex = s.find_first_of(delim, startIndex);
			nextStartIndex = endIndex + 1;
		}

		ret.push_back(s.substr(startIndex, endIndex == std::string::npos ? endIndex : endIndex - startIndex));
		if (endIndex == std::string::npos)
			break;

		startIndex = nextStartIndex;
	}

	return ret;
}

static void parseAirports() {
	std::ifstream f(g_airportsFile);
	for (std::string line; std::getline(f, line);) {
		const auto fields = parseCSVLine(line, ',');
		AirportInfo airport;
		airport.airportName = fields[1];
		airport.city = fields[2];
		airport.country = fields[3];
		airport.code = fields[4];

		const auto & icao = fields[5];
		g_airports[icao] = std::move(airport);
	}
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
			if (shouldDownloadAirports())
				downloadAirports();
			parseAirports();

			while (_em.running) {
				const auto s = runProcess(makeCurlCommand("https://opensky-network.org/api/states/all", {
					{ "lamin", "48.749256" },
					{ "lomin", "2.401905" },
					{ "lamax", "48.775333" },
					{ "lomax", "2.525670" }
				}));

				if (!_em.running)
					break;

				g_lastResponse = s;
				if (!putils::json::accept(s))
					continue;

				const auto json = putils::json::parse(s);
				std::vector<PlaneInfo> planes;

				for (const auto & state : json["states"]) {
					PlaneInfo plane;
					plane.callSign = state[1].get<std::string>();
					plane.country = state[2].get<std::string>();

					const auto t = ::time(nullptr);
					const auto s = runProcess(makeCurlCommand("https://opensky-network.org/api/routes", {
						{ "callsign", plane.callSign }
					}));
					if (!_em.running)
						break;

					if (!putils::json::accept(s))
						continue;

					const auto json = putils::json::parse(s);
					plane.json = json;

					plane.company = json["operatorIata"].get<std::string>();
					plane.flightNumber = putils::toString(json["flightNumber"].get<int>());

					plane.origin = json["route"][0].get<std::string>();
					plane.destination = json["route"][1].get<std::string>();

					planes.push_back(std::move(plane));

					std::unique_lock<std::mutex> l(g_planesMutex);
					g_planes = planes;
				}
			}
		}
		catch (const std::exception & e) {
			std::cerr << e.what() << '\n';
		}
	});
}