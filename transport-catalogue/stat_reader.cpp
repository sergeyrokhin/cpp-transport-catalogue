#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

namespace transport {

    ostream& operator<<(ostream& os, const Stop_Stop& Stop_Stop) {
        return os << "Stop: " << Stop_Stop.first->name_ << " to Stop: " << Stop_Stop.second->name_;
    }


    ostream& operator<<(ostream& os, const Stop& stop) {
        return os  << "Stop: " << stop.name_ << " <" << stop.geo_.lat << " , " << stop.geo_.lng << ">" << endl;
    }

    ostream& operator<<(ostream& os, const Bus& bus) {
        os << "Bus: " << bus.name_ << " " << bus.bus_route_.begin()->stop_stop.first->name_;
        for (auto& route : bus.bus_route_)
        {
            os << " " << (route.reversible ? '-' : '>') << " " << route.stop_stop.second->name_;
        }
        return os << endl;
    }

    tuple<double, double, double> DistCalculate(BusDepot& depot, Stop_Stop stop_stop, bool reversible = false) {
        auto g_length = ComputeDistance(stop_stop.first->geo_, stop_stop.second->geo_);
        auto length_l = depot.GetFixDistance(stop_stop);
        double length = length_l ? length_l : g_length;
        auto length_r = length;
        if (reversible)
        {
            auto length_l = depot.GetFixDistance(stop_stop);
            length_r = length_l ? length_l : length;
        }
        return { length, g_length, length_r };
    }

    void ReportBus(BusDepot& depot, string_view name, std::ostream& output) {
        output << "Bus " << name << ": ";
        auto ptr_bus = depot.FindBus(name);
        if (ptr_bus == nullptr)
        {
            output << "not found" << endl;
        }
        else
        {
            unordered_set<Stop*> count_unique; //только для того, чтоб посчитать уникальные
            double route_length = 0;
            double geo_length = 0;
            int count_stop = 1;
            count_unique.insert((ptr_bus->bus_route_.begin())->stop_stop.first);
            for (auto& step : ptr_bus->bus_route_) {
                count_unique.insert(step.stop_stop.second);
                auto [length, g_length, length_r] = DistCalculate(depot, step.stop_stop, step.reversible);
                route_length += length;
                geo_length += g_length;
                ++count_stop;
                if (step.reversible)
                {
                    ++count_stop;
                    route_length += length_r;
                    geo_length += g_length;
                }
            }
            output << count_stop << " stops on route, " << count_unique.size() <<
                " unique stops, " << route_length << " route length, " << (route_length / geo_length) << " curvature" << endl;
        }
    }
    void ReportStop(BusDepot& depot, string_view name, std::ostream& output) {
        output << "Stop " << name << ": ";
        auto ptr_stop = depot.FindStop(name);
        if (ptr_stop == nullptr)
        {
            output << "not found";
        }
        else
        {
            deque<string_view> selected_bases;
            auto& buses = depot.GetAllBuses();
            for_each(buses.begin(), buses.end(), [ptr_stop, &selected_bases, name](auto& bus) {
                if (any_of(bus.bus_route_.begin(), bus.bus_route_.end(),
                [ptr_stop](auto& step) {
                        if (ptr_stop == step.stop_stop.first) return true;
            return (ptr_stop == step.stop_stop.second);
                    }))
                { //добавляем
                    selected_bases.push_back(bus.name_);
                }
                });
            sort(selected_bases.begin(), selected_bases.end());
            //вывод
            if (selected_bases.size()) {
                output << "buses";
                for (auto& name : selected_bases) {
                    output << " " << name;
                }
            }
            else output << "no buses";
        }
        output << endl;
    }

    void Report(BusDepot& depot, std::istream& input, std::ostream& output) {

        string string_count;
        getline(input, string_count);
        int count = std::stoi(string_count);
        for (string line; getline(input, line), count > 0; --count) {
            auto [prefix, text] = Split(line, ' ');

            auto name = LRstrip(text);

            if (LRstrip(prefix) == "Bus") 
                ReportBus(depot, name, output);
            else // if (prefix == "Stop") { //command == "Stop"
                ReportStop(depot, name, output);
        }
    }

    void ReportBusDepot(BusDepot& depot, std::ostream& output) {

        //output.precision(1);
        //output.setf(ios::fixed);

        output << "=========== Stops ===============" << endl;
        const StopIndex& stops = depot.GetStopIndex();
        for (auto& stop : stops)
        {
            output << *stop.second;
        }
        output << "=========== Stop to Stop ========" << endl;
        const Distances& stop2stop = depot.GetDistances();
        for (auto& stop_stop : stop2stop)
        {
            output << stop_stop.first << " dist :" << stop_stop.second << "m" << endl;;
        }
        output << "============ Bus ================" << endl;
        const BusIndex& buses = depot.GetBusIndex();
        for (auto& bus : buses)
        {
            output << *(bus.second);
        }
        output << "==========================" << endl;
    }
}