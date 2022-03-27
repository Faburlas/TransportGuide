#pragma once

#include <string_view>
#include <string>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <vector>
#include <iostream>
#include <memory>


#include "geo.h"
#include "domain.h"

using namespace tg::detail;

namespace tg {

	class TransportGuide {

		using NameToBus = std::unordered_map<std::string, Bus*>;
	public:
		void AddBus(std::string name, std::vector<std::string>& stops, bool isCircle);

		void AddStop(std::string name, Coordinates coordinates);

		const Bus* FindBus(std::string name)const;

		const Stop* FindStop(std::string name) const;

		const std::unordered_set<Bus*> FindAllBusesToStop(const Stop* stop);

		bool IsStopDontHaveBuses(const std::shared_ptr<Stop> stop);

		int GetRealStopsDistance(const Stop* stopA, const Stop* stopB) const;

		void SetStopsDistance(const std::string stop_name_A, const std::string stop_name_B, int distance);

		class TwoStopHasher {
		public:
			size_t operator()(const std::pair<const Stop*, const Stop*> stops) const {
				std::hash<const void*> phasher;
				size_t a = phasher(stops.first);
				size_t b = phasher(stops.second);
				return (a + b) * (a + b + 1) / 2 + b;
			}
		};

		std::list<Stop> GetStops() ;
		std::list<Bus> GetBuses() ;

		Stops GetStopsSharedPtrs() ;
		Buses GetBusesSharedPtrs() ;


		bool HasStop(std::string name) const;
	private:
		// key - name of the stop, value - ptr to Stop
		std::unordered_map<std::string, Stop*> name_to_stop_;
		//key - name of the bus, value - ptr to Bus
		NameToBus name_to_route_;
		// key - Stop, value - set of Buses
		std::unordered_map<const Stop*, std::unordered_set<Bus*>> stop_to_routes_;
		//key - two stops pair , value - distance
		std::unordered_map<std::pair<const Stop*, const Stop*>, int, TwoStopHasher> stops_distance;
		std::list<Stop> stops_;
		std::list<Bus> buses_;

		Stops stops_ptr_set_;
		Buses buses_ptr_set_;

		NameToBus GetNameToBus() {
			return name_to_route_;
		}
	};
}
