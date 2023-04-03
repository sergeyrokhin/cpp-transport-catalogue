#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "transport_router.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport {
	void Load(TransportCatalogue& catalogue, const json::Document& doc);
	void Report(const TransportCatalogue& catalogue, const json::Document& doc, std::ostream& output = std::cout);
	//расстояние с учетом справочной информации
	//расстояние географическое

	void ReportBuscatalogue(TransportCatalogue& catalogue, std::ostream& output = std::cout);
	std::ostream& operator<<(std::ostream& os, const Stop& stop);
	std::ostream& operator<<(std::ostream& os, const Bus& bus);

	using namespace std::literals;
	static constexpr std::string_view ERROR_TEXT = "error_message"sv;
	static constexpr std::string_view NOT_FOUND_TEXT = "not found"sv;
	static constexpr std::string_view TYPE_TEXT = "type"sv;
	static constexpr std::string_view BUS_TEXT = "Bus"sv;


    template <typename Weight>
    json::Dict ReportRoute(const TransportCatalogue& catalogue, const RouterProperty& router_property, std::string_view from_stop, std::string_view to_stop) {
        json::Dict result;

        using namespace std;

        static RouterMap<Weight> router_map(catalogue, router_property);

        auto router_info = router_map.router_.BuildRoute(router_map.vertexes_.at({ *catalogue.FindStop(from_stop) }),
            router_map.vertexes_.at({ *catalogue.FindStop(to_stop) }));
        //посчитан, формируем результат

        static constexpr string_view BUS__TEXT = "bus"sv;
        static constexpr string_view SPAN_COUNT_TEXT = "span_count"sv;
        static constexpr string_view TOTAL_TIME_TEXT = "total_time"sv;
        static constexpr string_view TIME_TEXT = "time"sv;
        static constexpr string_view WAIT_TEXT = "Wait"sv;
        static constexpr string_view ITEMS_TEXT = "items"sv;
        static constexpr string_view STOP_NAME_TEXT = "stop_name"sv;


        if (router_info == nullopt)
        {
            result.insert({ string(ERROR_TEXT), string(NOT_FOUND_TEXT) });
        }
        else
        {
            result.insert({ string(TOTAL_TIME_TEXT), router_info.value().weight });
            auto& edges = router_info.value().edges;
            json::Array spans_result;
            json::Dict span;
            for (const auto& edge_id : edges)
            {
                auto& edge = router_map.edge_mileage_.at(edge_id);
                auto& vertex_from = router_map.vertex_list_[edge.edge.from];

                span.insert({ string(TYPE_TEXT), string(WAIT_TEXT) });
                span.insert({ string(TIME_TEXT), edge.mileage.duration_boarding });
                span.insert({ string(STOP_NAME_TEXT), vertex_from.stop.name_ });
                spans_result.push_back(move(span));
                span.clear();

                span.insert({ string(TYPE_TEXT), string(BUS_TEXT) });
                span.insert({ string(BUS__TEXT), edge.bus->name_ });
                span.insert({ string(TIME_TEXT), edge.mileage.duration - edge.mileage.duration_boarding });
                span.insert({ string(SPAN_COUNT_TEXT), edge.mileage.span_count });
                spans_result.push_back(move(span));
                span.clear();

            }
            result.insert({ string(ITEMS_TEXT), move(spans_result) });
        }
        return result;
    }
}
