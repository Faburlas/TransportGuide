#include <unordered_set>
#include <vector>
#include <optional>
#include <stdexcept>

#include "transport_catalogue.h"

namespace tg {

	//add stop
	void TransportGuide::AddStop(std::string name, Coordinates coordinates) {
		if (name_to_stop_.count(name) != 0) {
			name_to_stop_.at(name)->coordinates = std::move(coordinates);
		}
		else
		{
			stops_.push_back({ name, coordinates });
			name_to_stop_[name] = &stops_.back();
			stops_ptr_set_.insert(std::make_shared<Stop>(stops_.back()));
		}
	}

	//add bus
	void TransportGuide::AddBus(std::string name, std::vector<std::string>& stop_names, bool isCircle) {
		std::vector<const Stop*> stops;
		stops.reserve(stop_names.size());

		for (auto& stop : stop_names) {
			if (name_to_stop_.count(stop) == 0) {
				//Перед тем как добавить автобус
				//Нужно чтобы были добавлены все остановки, даже те
				//координаты которых мы не знаем
				AddStop(stop, { 0,0 });
			}
			const Stop* stopPtr = name_to_stop_.at(stop);
			stops.push_back(stopPtr);
		}

		if (!isCircle && stop_names.size() > 0) {
			for (int i = stops.size() - 2; i >= 0; --i) {
				stops.push_back(stops[i]);
			}
		}

		buses_.push_back({ name, stops, isCircle });
		this->name_to_route_.insert({ name, &buses_.back() });
		buses_ptr_set_.insert(std::make_shared<Bus>(buses_.back()));

		for (auto stop_pointer : stops) {
			stop_to_routes_[stop_pointer].insert(&buses_.back());
		}
	}

	//find bus by name
	const Bus* TransportGuide::FindBus(std::string name) const {
		if (name_to_route_.find(name) != name_to_route_.end()) {
			return name_to_route_.at(name);
		}
		else {
			return nullptr;
		}
	}

	//find stop by name
	const Stop* TransportGuide::FindStop(std::string name) const {
		if (name_to_stop_.find(name) != name_to_stop_.end()) {
			return name_to_stop_.at(name);
		}
		else {
			return nullptr;
		}
	}

	//find all buses that have given stop in route
	const std::unordered_set<Bus*> TransportGuide::FindAllBusesToStop(const Stop* stop) {
		if (stop_to_routes_.find(stop) != stop_to_routes_.end()) {
			return stop_to_routes_.at(stop);
		}
		else
		{
			return std::unordered_set<Bus*>();
		}
	}

	bool TransportGuide::IsStopDontHaveBuses(const std::shared_ptr<Stop> stop) {
		auto search_for_stop = name_to_stop_.find(stop.get()->name);

		if (search_for_stop != name_to_stop_.end()) {
			if (stop_to_routes_.find(search_for_stop->second) != stop_to_routes_.end()) {
				return true;
			}
			else {
				return false;
			}
		}
		return false;
	}


	int TransportGuide::GetRealStopsDistance(const Stop* stop_A, const Stop* stop_B) const {
		std::pair<const Stop*, const Stop*> pairAB{ stop_A, stop_B };
		if (stops_distance.count(pairAB) > 0) {
			return stops_distance.at(pairAB);
		}
		else {
			if (stop_A == stop_B) {
				return 0;
			}
			std::pair<const Stop*, const Stop*> pairBA{ stop_B, stop_A };
			if (stops_distance.count(pairBA) > 0) {
				return stops_distance.at(pairBA);
			}
		}
		return 0;
	}

	//Add stop distance between A and B
	void TransportGuide::SetStopsDistance(const std::string stop_name_A, const std::string stop_name_B, int distance) {
		auto stopA = FindStop(stop_name_A);
		auto stopB = FindStop(stop_name_B);
		if (stopA == nullptr || stopB == nullptr)
			return;

		stops_distance[std::pair<const Stop*, const Stop*> {stopA, stopB}] = distance;
	}

	bool TransportGuide::HasStop(std::string name) const
	{
		return name_to_stop_.count(name) != 0;
	}

	std::list<Bus> TransportGuide::GetBuses() {
		return buses_;
	}

	std::list<Stop> TransportGuide::GetStops() {
		return stops_;
	}

	Stops TransportGuide::GetStopsSharedPtrs() {
		return stops_ptr_set_;
	}

	Buses TransportGuide::GetBusesSharedPtrs() {
		return buses_ptr_set_;
	}
}