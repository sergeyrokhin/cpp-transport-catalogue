#pragma once
#include "geo.h"

#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <iostream>



namespace transport {

	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator () (std::pair<T1, T2> const& pair) const {
			std::size_t h1 = std::hash<T1>()(pair.first);
			std::size_t h2 = std::hash<T2>()(pair.second);
			return h1 + h2 * 13;
		}
	};

	struct Stop;
	struct Step;
	struct Bus;

	using StopIndex = std::unordered_map<std::string_view, Stop*>;
	using BusIndex = std::unordered_map<std::string_view, Bus*>;
	using Stops = std::deque<Stop>;
	using Buses = std::deque<Bus>;
	using Stop_Stop = std::pair<Stop*, Stop*>;

	using Distances = std::unordered_map<Stop_Stop, size_t, pair_hash>;


	struct Stop {
		Stop(const std::string_view name) : name_(name.substr()) { geo_ = { 0, 0 }; }
		std::string name_;
		Coordinates geo_;
		bool operator==(const Stop& rh) { return name_ == rh.name_; }
	};

	struct Step {
		Stop_Stop stop_stop;
		bool reversible;
	};

	struct Bus {
		Bus(const std::string_view name) : name_(name.substr()) {}
		std::string name_;
		std::deque<Step> bus_route_;
		bool operator==(const Bus& rh) { return name_ == rh.name_; }
	};

	class BusDepot {

	public:
		std::string_view GetWordView(std::string_view name);
		Stop* FindStop(std::string_view name);
		Stop& GetStop(std::string_view name);
		Stop& GetStop(std::string_view name, double latitude, double longitude);
		Bus* FindBus(std::string_view name);
		Bus& GetBus(std::string_view name);
		void AddStep(Bus& bus, std::string_view stop_name_a, std::string_view stop_name_b, bool reversible);
		void AddStep(Bus& bus, Stop_Stop stop_stop, bool reversible);
		size_t GetFixDistance(Stop_Stop stop_stop);
		void AddDistance(const Stop_Stop stop_stop, size_t distance);
		const BusIndex& GetBusIndex();
		const StopIndex& GetStopIndex();
		const Buses& GetAllBuses();
		const Stops& GetAllStops();
		const Distances& GetDistances();

	private:		
		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;
		StopIndex ind_stops_;
		BusIndex ind_buses_;
		Distances all_distances_;
	};

}