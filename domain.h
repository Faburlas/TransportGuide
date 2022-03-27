#pragma once

#include <vector>
#include <string>
#include <memory>
#include <set>

#include "geo.h"

//Bus Stop structure
struct Stop {
	std::string name;
	tg::detail::Coordinates coordinates;
};

//Bus structure
struct Bus {
	std::string name;
	std::vector<const Stop*> stops;
	bool isCircle;
};

struct BusStatistics {
	int stops = 0;
	int unique_stops = 0;
	int route_length = 0;
	double curvature = 1.0;
};


struct BusComparator {
	bool operator()(std::shared_ptr<Bus> lhs,
		std::shared_ptr<Bus> rhs) const {
		return lexicographical_compare(
			lhs->name.begin(), lhs->name.end(),
			rhs->name.begin(), rhs->name.end());
	}
};

struct StopComparator {
	bool operator()(std::shared_ptr<Stop> lhs,
		std::shared_ptr<Stop> rhs) const {
		return lexicographical_compare(
			lhs->name.begin(), lhs->name.end(),
			rhs->name.begin(), rhs->name.end());
	}
};

using Buses = std::set<std::shared_ptr<Bus>, BusComparator>;
using Stops = std::set<std::shared_ptr<Stop>, StopComparator>;