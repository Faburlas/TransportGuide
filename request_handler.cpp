#include "request_handler.h"

RequestHandler::RequestHandler(const tg::TransportGuide& db) : db_(db) {}

std::optional<BusStatistics> RequestHandler::GetBusStat(const std::string& bus_name) const {

	int stops_on_route;
	double coords_length = 0;
	double length = 0;
	const Stop* previous = nullptr;
	std::unordered_set<std::string> unique_stops;

	const Bus* bus = db_.FindBus(bus_name);

	if (bus == nullptr)
		return std::nullopt;

	for (const Stop* current : bus->stops) {
		unique_stops.insert(current->name);
		if (previous) {
			coords_length += ComputeDistance(previous->coordinates, current->coordinates);
			length += db_.GetRealStopsDistance(previous, current);
		}
		previous = current;
	}

	for (auto stop : bus->stops)
		unique_stops.insert(stop->name);

	int unique_stops_amount = unique_stops.size();
	stops_on_route = bus->stops.size();


	return BusStatistics{ stops_on_route, unique_stops_amount, (int)length, length / coords_length };
}