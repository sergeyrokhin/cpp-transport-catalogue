#include <algorithm>
#include <iostream>

#include "transport_catalogue.h"

using namespace std;

namespace transport {

    //******* Stop
    void TransportCatalogue::AddDistance(const Stop_Stop Stop_Stop, size_t distance) {
        all_distances_[Stop_Stop] = distance;
    }

    Stop* TransportCatalogue::FindStop(std::string_view name) const {
        auto it_stop = ind_stops_.find(name);
        if (it_stop == ind_stops_.end())
        {
            return nullptr;
        }
        return (*it_stop).second;
    }

    Stop& TransportCatalogue::GetAddStop(std::string_view name) {
        auto stop_ptr = FindStop(name);
        if (stop_ptr == nullptr) {
            auto& new_stop = all_stops_.emplace_back(Stop{ name });
            ind_stops_[static_cast<std::string_view>(new_stop.name_)] = &new_stop;
            return new_stop;
        }
        else return *stop_ptr;
    }

    Stop& TransportCatalogue::GetAddStop(std::string_view name, double latitude, double longitude) {
        auto& stop = GetAddStop(name);
        stop.geo_ = { latitude,longitude };
        return stop;
    }
    void TransportCatalogue::SetStop(Stop& stop, double latitude, double longitude) {
        stop.geo_ = { latitude,longitude };
    }
    //**************
    const StopIndex& TransportCatalogue::GetStopIndex() const { return ind_stops_; }
    const BusIndex& TransportCatalogue::GetBusIndex() const { return ind_buses_; }
    const Buses& TransportCatalogue::GetAllBuses() const { return all_buses_; }
    const Stops& TransportCatalogue::GetAllStops() const { return all_stops_; }
    const Distances& TransportCatalogue::GetDistances() const { return all_distances_; }

    //********* Bus
    Bus* TransportCatalogue::FindBus(const std::string_view name) const {
        auto it = ind_buses_.find(name);
        if (it == ind_buses_.end())
        {
            return nullptr;
        }
        return (*it).second;
    }

    Bus& TransportCatalogue::GetAddBus(std::string_view name) {
        auto bus_ptr = FindBus(name);
        if (bus_ptr == nullptr) {
            auto& new_bus = all_buses_.emplace_back(Bus{ name });
            ind_buses_[new_bus.name_] = &new_bus;
            return new_bus;
        }
        return *bus_ptr;
    }

    void TransportCatalogue::AddStep(Bus& bus, std::string_view stop_name) {
        bus.bus_route_.push_back(&GetAddStop(stop_name));
    }

    void TransportCatalogue::AddStep(Bus& bus, Stop* stop) {
        bus.bus_route_.push_back(stop);
    }

    size_t TransportCatalogue::GetFixDistance(const Stop_Stop stop_stop) const {
        auto it = all_distances_.find(stop_stop);
        if (it != all_distances_.end()) return it->second;
        it = all_distances_.find({ stop_stop.second, stop_stop.first });
        if (it != all_distances_.end()) return it->second;
        return 0;
    }

} //namespace transport