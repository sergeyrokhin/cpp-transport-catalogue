#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

#include <iostream>
#include <string>
#include <algorithm>

using namespace std;

namespace transport {

    tuple<double, double, double> DistCalculate(BusDepot& depot, Stop* stop_a, Stop* stop_b, bool reversible = false) {
        auto g_length = ComputeDistance(stop_a->geo_, stop_b->geo_);
        auto length_l = depot.GetFixDistance(stop_a, stop_b);
        double length = length_l ? length_l : g_length;
        auto length_r = length;
        if (reversible)
        {
            auto length_l = depot.GetFixDistance(stop_b, stop_a);
            length_r = length_l ? length_l : length;
        }
        return { length, g_length, length_r };
    }

    void Report(BusDepot& depot, std::istream& input) {

        string string_count;
        getline(input, string_count);
        int count = std::stoi(string_count);
        for (string line; getline(input, line), count > 0; --count) {
            auto [prefix, text] = Split(line, ' ');

            auto name = LRstrip(text);

            if (LRstrip(prefix) == "Bus")
            {

                cout << "Bus " << name << ": ";
                auto ptr_bus = depot.FindBus(name);
                if (ptr_bus == nullptr)
                {
                    cout << "not found" << endl;
                }
                else
                {
                    unordered_set<Stop*> count_unique; //только для того, чтоб посчитать уникальные
                    double route_length = 0;
                    double geo_length = 0;
                    int count_stop = 1;
                    count_unique.insert((ptr_bus->bus_route_.begin())->stop_a_);
                    for (auto& step : ptr_bus->bus_route_) {
                        count_unique.insert(step.stop_b_);
                        auto [length, g_length, length_r] = DistCalculate(depot, step.stop_a_, step.stop_b_, step.reversible);
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
                    cout << count_stop << " stops on route, " << count_unique.size() <<
                        " unique stops, " << route_length << " route length, " << (route_length / geo_length)  << " curvature" << endl;
                }
            }
            else if (LRstrip(prefix) == "Stop") { //command == "Stop"
                cout << "Stop " << name << ": ";
                auto ptr_stop = depot.FindStop(name);
                if (ptr_stop == nullptr)
                {
                    cout << "not found";
                }
                else
                {
                    deque<string_view> selected_bases;
                    auto& buses = depot.GetAllBuses();
                    for_each(buses.begin(), buses.end(), [ptr_stop, &selected_bases, name](auto& bus) {
                        if (any_of(bus.bus_route_.begin(), bus.bus_route_.end(), 
                            [ptr_stop](auto& step) {
                                if (ptr_stop == step.stop_a_) return true;
                                return (ptr_stop == step.stop_b_);
                            }))
                        { //добавляем
                            selected_bases.push_back(bus.name_);
                        }
                    });
                    sort(selected_bases.begin(), selected_bases.end());
                    //вывод
                    if (selected_bases.size()) {
                        cout << "buses";
                        for (auto& name : selected_bases) {
                            cout << " " << name;
                        }
                    }
                    else cout << "no buses";
                }
                cout << endl;
            }
            else {
            }
        }
    }

    void ReportBusDepot(BusDepot& depot) {

        cout.precision(1);
        cout.setf(ios::fixed);

        cout << "=========== Stops ===============" << endl;
        const StopIndex& stops = depot.GetStopIndex();
        for (auto& stop : stops)
        {
            cout << *stop.second << endl;
        }
        cout << "=========== Bus ===============" << endl;
        const BusIndex& buses = depot.GetBusIndex();
        for (auto& bus : buses)
        {
            cout << *(bus.second) << endl;
        }
    }
}