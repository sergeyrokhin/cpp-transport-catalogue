#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "graph.h"
#include "router.h"
#include "json_reader.h"

#include <string_view>
#include <unordered_map>
#include <algorithm>

namespace transport {
	using Weight = double;
	using Graph = graph::DirectedWeightedGraph<Weight>;

	json::Dict ReportRoute(const TransportCatalogue& depot, const json::Document& doc, std::string_view from_stop, std::string_view to_stop);

	struct Mileage {
		Weight duration_boarding = 0;
		Weight duration = 0;
		int span_count = 0;
	};

	struct Vertex {
		const Stop& stop;
		bool operator == (const Vertex& other) const {
			return &stop == &(other.stop);
		}
	};

	struct vertex_hash {
		std::size_t operator () (Vertex vertex) const {
			return std::hash<std::string>()(vertex.stop.name_);
		}
	};

	using IndexStops = std::unordered_map<const Vertex, graph::VertexId, vertex_hash>;

	struct Edge {
		graph::VertexId from;
		graph::VertexId to;
	};

	struct EdgeMileage {
		const Bus* bus = nullptr;
		Edge edge;
		Mileage mileage;
	};
	using EdgeID_EdgeMileage = std::vector<EdgeMileage>; //придется запоминать, т.к. решение по графу не возвращает детализацию

	void AddEdgeInGraph(Graph& graph, EdgeID_EdgeMileage& id_edge_mileage, const EdgeMileage& edgemileage);

	template <typename Weight>
	class RouterMap {
	public:
		RouterMap(const TransportCatalogue& depot, const RouterProperty& router_prop);

		IndexStops vertexes_; //Поиск ID вершин
		EdgeID_EdgeMileage edge_mileage_; //подробная информация о ребрах по ID по поряюку
		std::vector<Vertex> vertex_list_; //список, по поряюку их VertexID
		RouterProperty router_prop_; //параметры для расчета времени (скорость и время ожидания)
		graph::Router<Weight> router_;
	};

	struct StopBuildup {
		graph::VertexId vertex_id;
		Mileage mileage = {};
	};

	//чтоб исключить дублирование кода используется шаблонная функция
	// т.к. прямой итератор и реверсный невозможно передавать одним параметром, 
	//это в свою очередь требует большого количества необходимых параметров
	//но действительно на два параметра сократил, заменив обобщающим объектом.
	//
	template <typename It>
	void FillGraphRoutes(Graph& graph, RouterMap<Weight>& router_map, const TransportCatalogue& depot, 
		const Bus& bus, const It first_stop, const It last_stop)
	{
		std::vector<StopBuildup> stop_buildup; //накапливаем расстояние
		Stop* past_stop = nullptr; //будем замерять расстояние от прошлой остановки
		int number = 0;
		for (auto stop_it = first_stop; stop_it != last_stop; stop_it++)
		{
			graph::VertexId vertex_stop_id = router_map.vertexes_.at({ *(*stop_it) });

			//// + сразу, чтоб проверять не последняя ли это остановка
			if (number++ != 0)//это не первая остановка
			{
				auto [lenght, _] = DistCalculate(depot, { past_stop, *stop_it }); // считаем расстояние от прошлой остановки
				auto router_time = lenght / router_map.router_prop_.bus_velocity_;

				for (auto& buildup : stop_buildup)
				{
					//посчитаем этот участок
					++buildup.mileage.span_count;
					buildup.mileage.duration += router_time;

					//приехал, вышел (с каждой предыдущей)
					AddEdgeInGraph(graph, router_map.edge_mileage_,
						{ &bus, { buildup.vertex_id, vertex_stop_id }, buildup.mileage }
					);
				}
			}
			stop_buildup.push_back({ { vertex_stop_id }, {router_map.router_prop_.bus_wait_time_, router_map.router_prop_.bus_wait_time_} });
			past_stop = *stop_it; //поехали на следующую
		}
	}
	
	//depot - перевод депо, автобусное депо
	//прошу разрешить оставить, чтоб не менять другие файлы
	template <typename Weight>
	Graph& BuildGraph(RouterMap<Weight>& router_map, const TransportCatalogue& depot) {

		//создание вершин графа
		auto& stops = depot.GetAllStops();
		for (auto& stop : stops)
		{
			router_map.vertex_list_.push_back({stop});
			router_map.vertexes_.insert({ {stop}, router_map.vertexes_.size() });
		}
		static Graph graph(router_map.vertexes_.size()); //по количеству вершин

		//создание рёбер графа
		auto& buses = depot.GetAllBuses();
		for (auto& bus : buses)
		{
			FillGraphRoutes(graph, router_map, depot, bus, bus.bus_route_.begin(), bus.bus_route_.end());

			if (!bus.roundtrip_) //не круговой, будет двигаться в обратную сторону
			{
				FillGraphRoutes(graph, router_map, depot, bus, bus.bus_route_.rbegin(), bus.bus_route_.rend());
			}
		}

		return graph;
	}

	template <typename Weight>
	RouterMap<Weight>::RouterMap(const TransportCatalogue& depot, const RouterProperty& router_prop) :
		vertexes_(), edge_mileage_(), vertex_list_(), router_prop_(router_prop), router_(BuildGraph(*this, depot))
	{

	}


	template <typename Weight>
	json::Dict ReportRoute(const TransportCatalogue& depot, const RouterProperty& router_prop, std::string_view from_stop, std::string_view to_stop) {
		json::Dict result;

		using namespace std;

		static RouterMap<Weight> router_map(depot, router_prop);

		auto router_info = router_map.router_.BuildRoute(router_map.vertexes_.at({ *depot.FindStop(from_stop) }),
														 router_map.vertexes_.at({ *depot.FindStop(to_stop) }));
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


} //namespace transport