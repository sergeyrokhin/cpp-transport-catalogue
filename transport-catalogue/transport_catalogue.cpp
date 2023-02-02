#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace transport {

//******* Stop
    void BusDepot::AddDistance(const Stop_Stop Stop_Stop, size_t distance) {
        all_distances_[Stop_Stop] = distance;
    }
    
    Stop* BusDepot::FindStop(std::string_view name) {
        auto it_stop = ind_stops_.find(name);
        if (it_stop == ind_stops_.end())
        {
            return nullptr;
        }
        return (*it_stop).second;
    }

    Stop& BusDepot::GetStop(std::string_view name) {
        auto stop_ptr = FindStop(name);
        if (stop_ptr == nullptr) {
            auto& new_stop = all_stops_.emplace_back(Stop{ name });
            ind_stops_[static_cast<std::string_view>(new_stop.name_)] = &new_stop;
            return new_stop;
        }
        else return *stop_ptr;
    }

    Stop& BusDepot::GetStop(std::string_view name, double latitude, double longitude) {
        auto& stop = GetStop(name);
        stop.geo_ = {latitude,longitude};
        return stop;
    }
//**************
    const StopIndex& BusDepot::GetStopIndex() { return ind_stops_; }
    const BusIndex& BusDepot::GetBusIndex() { return ind_buses_; }
    const Buses& BusDepot::GetAllBuses() { return all_buses_; }
    const Stops& BusDepot::GetAllStops() { return all_stops_; }
    const Distances& BusDepot::GetDistances() { return all_distances_; }

    //********* Bus
    Bus* BusDepot::FindBus(std::string_view name) {
        auto it = ind_buses_.find(name);
        if (it == ind_buses_.end())
        {
            return nullptr;
        }
        return (*it).second;
    }

    Bus& BusDepot::GetBus(std::string_view name) {
        auto bus_ptr = FindBus(name);
        if (bus_ptr == nullptr) {
            auto& new_bus = all_buses_.emplace_back(Bus{ name });
            ind_buses_[new_bus.name_] = &new_bus;
            return new_bus;
        }
        return *bus_ptr;
    }
    void BusDepot::AddStep(Bus& bus, std::string_view stop_name_a, std::string_view stop_name_b, bool reversible) {
        auto & stop_a = GetStop(stop_name_a);
        auto & stop_b = GetStop(stop_name_b);
        BusDepot::AddStep(bus, { &stop_a, &stop_b }, reversible);
    }
    void BusDepot::AddStep(Bus& bus, Stop_Stop stop_stop, bool reversible) {
        bus.bus_route_.push_back({ stop_stop, reversible });
    }
    size_t BusDepot::GetFixDistance(Stop_Stop stop_stop) {
        auto it = all_distances_.find(stop_stop);
        if (it != all_distances_.end()) return it->second;
        it = all_distances_.find({ stop_stop.second, stop_stop.first });
        if (it != all_distances_.end()) return it->second;
        return 0;
    }
}