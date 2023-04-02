#include <iostream>
#include <string>
#include <algorithm>
#include <sstream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "domain.h"

using namespace std;

namespace transport {

    ostream& operator<<(ostream& os, const Stop_Stop& stop_stop) {
        return os << "Stop: " << stop_stop.first->name_ << " to Stop: " << stop_stop.second->name_;
    }
    ostream& operator<<(ostream& os, const Stop& stop) {
        return os << "Stop: " << stop.name_ << " <" << stop.geo_.lat << " , " << stop.geo_.lng << ">\n";
    }
    ostream& operator<<(ostream& os, const Bus& bus) {
        os << "Bus: " << bus.name_;
        for (auto route : bus.bus_route_)
        {
            os << " " << (bus.roundtrip_ ? '>' : '-') << " " << route->name_;
        }
        return os << '\n';
    }

    static constexpr string_view CURVATURE_TEXT = "curvature"sv;
    static constexpr string_view ROUTE_LENGTH_TEXT = "route_length"sv;
    static constexpr string_view STOP_COUNT_TEXT = "stop_count"sv;
    static constexpr string_view UNIQUE_STOP_COUNT = "unique_stop_count"sv;
    static constexpr string_view BUSES_TEXT = "buses"sv;

    json::Dict ReportBus(const TransportCatalogue& depot, const string_view name) {
        json::Dict result;
        auto ptr_bus = depot.FindBus(name);

        if (ptr_bus == nullptr)
        {
            result.insert({ string(ERROR_TEXT), string(NOT_FOUND_TEXT) });
        }
        else
        {
            unordered_set<Stop*> count_unique; //только для того, чтоб посчитать уникальные
            double route_length = 0;
            double geo_length = 0;
            
            Stop* prev = nullptr;
            for (auto& step : ptr_bus->bus_route_) {
                count_unique.insert(step);
                auto distance = DistCalculate(depot, {prev, step}, ptr_bus->roundtrip_);
                prev = step;
                route_length += distance.length;
                geo_length += distance.g_length;
            }
            int count_stop = ptr_bus->bus_route_.size() ; // исходную остановку считаем.
            if (!ptr_bus->roundtrip_) count_stop = count_stop * 2 - 1; //некруговой, посчитаем и обратные шаги

            result.insert({ string(CURVATURE_TEXT), static_cast<double>(route_length / geo_length) });
            result.insert({ string(ROUTE_LENGTH_TEXT), route_length});
            result.insert({ string(STOP_COUNT_TEXT), count_stop });
            result.insert({ string(UNIQUE_STOP_COUNT), static_cast<int>(count_unique.size()) });
        }
        return result;
    }

    json::Dict  ReportStop(const TransportCatalogue& depot, const string_view name) {
        json::Dict result;
        auto ptr_stop = depot.FindStop(name);
        if (ptr_stop == nullptr)
        {
            result.insert({ string(ERROR_TEXT), string(NOT_FOUND_TEXT) });
        }
        else
        {
            
            deque<string_view> selected_buses;
            auto& buses = depot.GetAllBuses();
            for_each(buses.begin(), buses.end(), [ptr_stop, &selected_buses, name](auto& bus) {
                if (any_of(bus.bus_route_.begin(), bus.bus_route_.end(),
                [ptr_stop](auto& step) { return (ptr_stop == step);}))
                { //добавляем
                    selected_buses.push_back(bus.name_);
                }
                });
            sort(selected_buses.begin(), selected_buses.end());
            //вывод
            json::Array buses_result;
            if (selected_buses.size()) {
                for (auto& name : selected_buses) {
                    buses_result.push_back(std::string{name});
                }
            }
            //else buses_result.push_back("no buses");
            result.insert({ string(BUSES_TEXT), move(buses_result) });
        }
        return result;
    }

    void ReportBusDepot(const TransportCatalogue& depot, std::ostream& output) {

        //output.precision(1);
        //output.setf(ios::fixed);

        output << "=========== Stops ===============\n";
        const StopIndex& stops = depot.GetStopIndex();
        for (auto& stop : stops)
        {
            output << *stop.second;
        }
        output << "=========== Stop to Stop ========\n";
        const Distances& stop2stop = depot.GetDistances();
        for (auto& stop_stop : stop2stop)
        {
            output << stop_stop.first << " dist :" << stop_stop.second << "m\n";;
        }
        output << "============ Bus ================\n";
        const BusIndex& buses = depot.GetBusIndex();
        for (auto& bus : buses)
        {
            output << *(bus.second);
        }
        output << "==========================\n";
    }

    
    static constexpr string_view IS_ROUT_TEXT = "is_roundtrip"sv;
    static constexpr string_view STOPS_TEXT = "stops"sv;


    /// ///////////////////////

    void LoadBus(TransportCatalogue& depot, string_view name, const json::Dict& bus_json) {
        auto& new_bus = depot.GetAddBus(name);
        
        bool is_routing_bus = false;
        if (bus_json.count(string(IS_ROUT_TEXT)))
        {
            is_routing_bus = bus_json.at(string(IS_ROUT_TEXT)).AsBool();
        }
        new_bus.roundtrip_ = is_routing_bus;
        if (bus_json.count(string(STOPS_TEXT)))
        {
            auto& stops_array = bus_json.at(string(STOPS_TEXT)).AsArray();
            for (auto& stop_node : stops_array)
            {
                depot.AddStep(new_bus, &depot.GetAddStop(stop_node.AsString()));
            }

        }
    }

    static constexpr string_view LAT_TEXT = "latitude"sv;
    static constexpr string_view LONG_TEXT = "longitude"sv;
    static constexpr string_view DISTANCE_TEXT = "road_distances"sv;

    void LoadStop(TransportCatalogue& depot, string_view name, const json::Dict& bus_json) {
        auto& stop = depot.GetAddStop(name);
        if (bus_json.count(string(LAT_TEXT)) || bus_json.count(string(LONG_TEXT)))
        {
            depot.SetStop(stop, bus_json.at(string(LAT_TEXT)).AsDouble(), bus_json.at(string(LONG_TEXT)).AsDouble());
        }
        if (bus_json.count(string(DISTANCE_TEXT)))
        {
            auto& distances = bus_json.at(string(DISTANCE_TEXT)).AsDict();
            for (auto& dist  : distances)
            {
                depot.AddDistance({ &stop, &depot.GetAddStop(dist.first) }, dist.second.AsInt());
            }
        }
    }

    static constexpr string_view BASE_REQUEST_TEXT = "base_requests"sv;
    static constexpr string_view STAT_REQUEST_TEXT = "stat_requests"sv;
    static constexpr string_view NAME_TEXT = "name"sv;
    static constexpr string_view STOP_TEXT = "Stop"sv;
    static constexpr string_view ROUTE_TEXT = "Route"sv;

    static constexpr string_view FROM_TEXT = "from"sv;
    static constexpr string_view TO_TEXT = "to"sv;

    static constexpr string_view MAP_TEXT = "Map"sv;
    static constexpr string_view MAP_OUT_TEXT = "map"sv;
    static constexpr string_view ID_TEXT = "id"sv;
    static constexpr string_view ID_REQUEST_TEXT = "request_id"sv;


    void Load(TransportCatalogue& depot, const json::Document& doc) {

        auto& root = doc.GetRoot().AsDict();

        if (root.count(string(BASE_REQUEST_TEXT)))
        {
            auto& bus_depot_array = root.at(string(BASE_REQUEST_TEXT)).AsArray();

            for (auto& bus_depot : bus_depot_array)
            {
                auto& bus_stop = bus_depot.AsDict();
                if ( ! bus_stop.count(string(TYPE_TEXT)))  throw json::ParsingError("Bus Depot ITEM unknown type"s);
                if ( ! bus_stop.count(string(NAME_TEXT)))  throw json::ParsingError("Bus Depot ITEM unknown name"s);
                auto& type_item = bus_stop.at(string(TYPE_TEXT)).AsString();
                auto& name_item = bus_stop.at(string(NAME_TEXT)).AsString();
                if (type_item == BUS_TEXT)
                    LoadBus(depot, name_item, bus_stop);
                else if (type_item == STOP_TEXT)
                    LoadStop(depot, name_item, bus_stop);
                else
                    throw json::ParsingError("Bus Depot ITEM unknown type"s);
            }
        }
        else std::cerr << "BusDEPOT is empty";

    }

    json::Dict ReportMap(const transport::TransportCatalogue& depot, const json::Document& doc) {
        json::Dict result;
        std::ostringstream out;

        auto map_doc = BusDepotMap(depot, doc);
        map_doc.MapRender(out);

        result.insert({ string(MAP_OUT_TEXT), {out.str()}});
        return result;
    }

    void Report(const TransportCatalogue& depot, const json::Document& doc, std::ostream& output) {
        auto& root = doc.GetRoot().AsDict();
        if (root.count(string(STAT_REQUEST_TEXT)))
        {
            auto& request_array = root.at(string(STAT_REQUEST_TEXT)).AsArray();
            json::Array reply_array;

            for (auto& request : request_array)
            {
                auto& request_map = request.AsDict();
                auto& request_item = request_map.at(string(TYPE_TEXT)).AsString();

                json::Dict reply;

                if (request_item == BUS_TEXT) {
                    reply = ReportBus(depot, request_map.at(string(NAME_TEXT)).AsString());
                }
                else if (request_item == STOP_TEXT)
                {
                    reply = ReportStop(depot, request_map.at(string(NAME_TEXT)).AsString());
                }
                else if (request_item == MAP_TEXT)
                {
                    reply = ReportMap(depot, doc);
                }
                else if (request_item == ROUTE_TEXT)
                {
                    static RouterProperty router_prop(doc); //считаем скорость и время ожидания
                    reply = ReportRoute<Weight>(depot, router_prop,
                        request_map.at(string(FROM_TEXT)).AsString(),
                        request_map.at(string(TO_TEXT)).AsString()
                        );
                }

                else  throw json::ParsingError("REQUEST ITEM unknown type"s);

                reply.insert({ string(ID_REQUEST_TEXT), request_map.at(string(ID_TEXT)).AsInt() });
                reply_array.push_back(reply);
            }
            json::Print(json::Document(reply_array), output);
        }
    }

    static constexpr string_view ROUTER_SETTINGS_TEXT = "routing_settings"sv;
    static constexpr string_view BUS_VELOCITY_TEXT = "bus_velocity"sv;
    static constexpr string_view BUS_WAIT_TIME_TEXT = "bus_wait_time"sv;
    static constexpr double KM_H_TO_METR_MIN_CONVERTER = 1000. / 60;

    RouterProperty::RouterProperty(const json::Document& doc) {
        auto& root = doc.GetRoot().AsDict();
        if (root.count(ROUTER_SETTINGS_TEXT)) {
            auto& property_router = (*root.find(ROUTER_SETTINGS_TEXT)).second.AsDict();
            if (property_router.count(BUS_WAIT_TIME_TEXT)) bus_wait_time_ = (*property_router.find(BUS_WAIT_TIME_TEXT)).second.AsInt();
            if (property_router.count(BUS_VELOCITY_TEXT))  bus_velocity_ = (*property_router.find(BUS_VELOCITY_TEXT)).second.AsDouble()
                * KM_H_TO_METR_MIN_CONVERTER;
        }
        else {
            throw json::ParsingError("ROUTER_SETTINGS_TEXT not found"s);
        }

    }

}