#pragma once
#include <string>
#include <string_view>
#include <deque>
#include <unordered_map>
#include <unordered_set>
#include <iostream>

#include "geo.h"
#include "domain.h"

namespace transport {

	struct pair_hash {
		template <class T1, class T2>
		std::size_t operator () (std::pair<T1, T2> const& pair) const {
			std::size_t h1 = std::hash<T1>()(pair.first);
			std::size_t h2 = std::hash<T2>()(pair.second);
			return h1 + h2 * 13;
		}
	};

	using StopIndex = std::unordered_map<std::string_view, Stop*>;
	using BusIndex = std::unordered_map<std::string_view, Bus*>;
	using Stops = std::deque<Stop>;
	using Buses = std::deque<Bus>;
	using Stop_Stop = std::pair<Stop*, Stop*>;

	using Distances = std::unordered_map<Stop_Stop, size_t, pair_hash>;

	class TransportCatalogue {

	public:
		Stop* FindStop(std::string_view name);
		Stop& GetStop(std::string_view name);
		Stop& GetStop(std::string_view name, double latitude, double longitude);
		void SetStop(Stop& stop, double latitude, double longitude);
		Bus* FindBus(std::string_view name);
		Bus& GetBus(std::string_view name);
		void AddStep(Bus& bus, std::string_view stop);
		void AddStep(Bus& bus, Stop* stop);
		size_t GetFixDistance(Stop_Stop stop_stop);
		void AddDistance(const Stop_Stop stop_stop, size_t distance);
		const BusIndex& GetBusIndex();
		const StopIndex& GetStopIndex();
		const Buses& GetAllBuses() const;
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