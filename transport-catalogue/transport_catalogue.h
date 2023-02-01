#pragma once
#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

namespace transport {

	struct Stop;
	struct Step;
	struct Bus;

	typedef std::unordered_map<std::string_view, Stop*> StopIndex;
	typedef std::unordered_map<std::string_view, Bus*> BusIndex;
	typedef std::deque<Stop> Stops;
	typedef std::deque<Bus> Buses;
	typedef std::unordered_map<Stop*, long> Distances;


	struct Stop {
		std::string_view name_;
		Coordinates geo_;
		Distances distances_;
		bool operator==(const Stop& rh) { return name_ == rh.name_; }
	};

	struct Step {
		Stop* stop_a_;
		Stop* stop_b_;
		bool reversible;
	};

	struct Bus {
		std::string_view name_;
		std::deque<Step> bus_route_;
		Distances distances_;
		bool operator==(const Stop& rh) { return name_ == rh.name_; }
	};

	std::ostream& operator<<(std::ostream& os, Stop& stop);
	std::ostream& operator<<(std::ostream& os, Bus& bus);

	class BusDepot {

	public:
		std::string_view GetWordView(std::string_view name);
		Stop* FindStop(std::string_view name);
		Stop& GetStop(std::string_view name);
		Stop& GetStop(std::string_view name, double latitude, double longitude);
		Bus* FindBus(std::string_view name);
		Bus& GetBus(std::string_view name);
		void AddStep(Bus& bus, std::string_view stop_name_a, std::string_view stop_name_b, bool reversible);
		void AddStep(Bus& bus, Stop* stop_a, Stop* stop_b, bool reversible);
		long GetFixDistance(Stop* stop_a, Stop* stop_b);
		const BusIndex& GetBusIndex();
		const StopIndex& GetStopIndex();
		const Buses& GetAllBuses();
		const Stops& GetAllStops();

	private:		
		std::deque<std::string> all_words_;
		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;
		StopIndex ind_stops_;
		BusIndex ind_buses_;
	};

}