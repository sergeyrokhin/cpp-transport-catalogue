#include "transport_catalogue.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace transport {

    std::string_view BusDepot::GetWordView(std::string_view word_view) {
        auto it_words = find(all_words_.begin(), all_words_.end(), word_view);
        if (it_words == all_words_.end())
            return static_cast<std::string_view>(all_words_.emplace_back(string(word_view)));
        else 
            return static_cast<std::string_view>(*it_words);
    }

//******* Stop

    ostream& operator<<(ostream& os, Stop& stop) {
        os << "Stop: " << stop.name_ << " <" << stop.geo_.lat << " , " << stop.geo_.lng << ">" << endl;
        if (stop.distances_.size())
        {
            os << "dist";
            char comma = ':';
            for (auto& distance : stop.distances_)
            {
                os << comma << " " << distance.second << "m to " << distance.first->name_;
                comma = ',';
            }
            os << endl;
        }
        return os;
    }

    ostream& operator<<(ostream& os, Bus& bus) {
        os << "Bus: " << bus.name_ << " " << bus.bus_route_.begin()->stop_a_->name_;
        for (auto& route : bus.bus_route_)
        {
            os << " " << (route.reversible ? '-' : '>') << " " << route.stop_b_->name_;
        }
        return os << endl;
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
            Stop stop;
            stop.name_ = GetWordView(name);
            stop.geo_ = { 0, 0 };
            auto& new_stop = all_stops_.emplace_back(stop); //?????
            ind_stops_[stop.name_] = &new_stop;
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
            Bus bus;
            bus.name_ = GetWordView(name);
            auto& new_bus = all_buses_.emplace_back(bus); //??????
            ind_buses_[bus.name_] = &new_bus;
            return new_bus;
        }
        return *bus_ptr;
    }
    void BusDepot::AddStep(Bus& bus, std::string_view stop_name_a, std::string_view stop_name_b, bool reversible) {
        auto & stop_a = GetStop(stop_name_a);
        auto & stop_b = GetStop(stop_name_b);
        BusDepot::AddStep(bus, &stop_a, &stop_b,reversible);
    }
    void BusDepot::AddStep(Bus& bus, Stop* stop_a, Stop* stop_b, bool reversible) {
        bus.bus_route_.push_back({ stop_a, stop_b, reversible });
    }
    long BusDepot::GetFixDistance(Stop* stop_a, Stop* stop_b) {
        for (auto& dist : stop_a->distances_)
        {
            if (dist.first == stop_b) return dist.second;
        }
        for (auto& dist : stop_b->distances_)
        {
            if (dist.first == stop_a) return dist.second;
        }
        return 0;
    }
}