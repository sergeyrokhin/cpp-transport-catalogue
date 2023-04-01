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
	class RouterProperty
	{
	public:
		explicit RouterProperty(const json::Document& doc);
		Weight	bus_wait_time_ = 6;
		Weight	bus_velocity_ = 600; //приведенное к м/мин или 36 км/ч

	private:

	};


	struct Vertex {
		const Stop& stop;
		bool on_route; // на автобусе на маршруте (off - ожидание)
		bool operator == (const Vertex& other) const {
			return on_route == other.on_route && &stop == &(other.stop);
		}
	};

	struct vertex_hash {
		std::size_t operator () (Vertex const& pair) const {
			std::size_t h1 = std::hash<std::string>()(pair.stop.name_);
			return h1 + (pair.on_route ? 13 : 0);
		}
	};

	json::Dict ReportRoute(const TransportCatalogue& depot, const json::Document& doc, std::string_view from_stop, std::string_view to_stop);

	struct Mileage {
		Weight time = 0;
		int span_count = 0;
	};

	using IndexStops = std::unordered_map<const Vertex, graph::VertexId, vertex_hash>;
	struct StopBuildup {
		//Stop& stop; //остановка от которой ведется накопление
		graph::VertexId vertex_id;
		Mileage mileage = {};
	};

	struct Edge {
		graph::VertexId from;
		graph::VertexId to;
	};

	struct EdgeMileage {
		const Bus* bus = nullptr;
		Edge edge;
		Mileage mileage;
	};
	using EdgeIDEdgeMileage = std::map<graph::EdgeId, EdgeMileage>; //придется запоминать, т.к. решение по графу не возвращает детализацию

	void AddEdgeInGraph(Graph& graph, EdgeIDEdgeMileage& id_edge_mileage, const EdgeMileage& edgemileage);

	template <typename Weight>
	class RouterMap {
	public:
		RouterMap(const TransportCatalogue& depot, const json::Document& doc);

		IndexStops vertexes_; //Поиск вершин по ID
		EdgeIDEdgeMileage edge_mileage_; //подробная информация о ребрах по ID
		std::vector<Vertex> vertex_list_; //список, по поряюку их VertexID
		graph::Router<Weight> router_;
	};

	template <typename It>
	void FillGraphRoutes(Graph& graph, EdgeIDEdgeMileage& id_edge_mileage,
		const TransportCatalogue& depot, const IndexStops& vertexes,
		const Bus& bus, const It first, const It last, const RouterProperty& router_prop)
	{
		std::vector<StopBuildup> stop_buildup; //накапливаем расстояние
		Stop* past_stop = nullptr; //будем замерять расстояние от прошлой остановки
		size_t stop_count = distance(first, last); //чтоб отслеживать последнюю остановку
		int number = 0;
		for (auto stop_it = first; stop_it != last; stop_it++)
		{
			graph::VertexId vertex_bus_id = vertexes.at({ *(*stop_it), true });
			graph::VertexId vertex_stop_id = vertexes.at({ *(*stop_it), false });

			//// + сразу, чтоб проверять не последняя ли это остановка
			if (number++ == 0)//это первая остановка
			{
				//if (number < stop_count) //это не последняя остановка. добавим посадку
				//{
				AddEdgeInGraph(graph, id_edge_mileage,
					{ &bus, { vertex_stop_id, vertex_bus_id }, { router_prop.bus_wait_time_, 0 } }
				); //сесть на этот автобус здесь
			}
			else //была предыдущая остановка
			{
				if (number < stop_count) //это не последняя остановка. добавим посадку
				{
					AddEdgeInGraph(graph, id_edge_mileage,
						{ &bus, { vertex_stop_id, vertex_bus_id }, { router_prop.bus_wait_time_, 0 } }
					); //сесть на этот автобус здесь
				}
				auto [lenght, _] = DistCalculate(depot, { past_stop, *stop_it }); // считаем расстояние от прошлой остановки
				auto router_time = lenght / router_prop.bus_velocity_;

				for (auto& buildup : stop_buildup)
				{
					//посчитаем этот участок
					++buildup.mileage.span_count;
					buildup.mileage.time += router_time;

					//приехал, вышел (с каждой предыдущей)
					AddEdgeInGraph(graph, id_edge_mileage,
						{ &bus, { buildup.vertex_id, vertex_stop_id }, { buildup.mileage.time, buildup.mileage.span_count } }
					);
				}
			}
			stop_buildup.push_back({ vertex_bus_id });
			past_stop = *stop_it; //поехали на следующую
		}
	}
	

	template <typename Weight>
	Graph& BuildGraph(RouterMap<Weight>& router_map, const TransportCatalogue& depot, const json::Document& doc) {
		RouterProperty router_prop(doc); //считаем скорость и время ожидания

		auto& stops = depot.GetAllStops();
		for (auto& stop : stops)
		{
			router_map.vertex_list_.push_back({ stop, false });
			router_map.vertex_list_.push_back({ stop, true });
			router_map.vertexes_.insert({ { stop, false }, router_map.vertexes_.size() }); //стоит на остановке
			router_map.vertexes_.insert({ { stop, true }, router_map.vertexes_.size() }); //готов переместиться по маршруту на "стоит на остановке"
		}
		static Graph graph(router_map.vertexes_.size()); //по количеству вершин

		auto& buses = depot.GetAllBuses();
		for (auto& bus : buses)
		{
			FillGraphRoutes(graph, router_map.edge_mileage_, depot, router_map.vertexes_, bus, bus.bus_route_.begin(), bus.bus_route_.end(), router_prop);

			if (!bus.roundtrip_) //не круговой, будет двигаться в обратную сторону
			{
				FillGraphRoutes(graph, router_map.edge_mileage_, depot, router_map.vertexes_, bus, bus.bus_route_.rbegin(), bus.bus_route_.rend(), router_prop);
			}
		}

		return graph;
	}

	template <typename Weight>
	RouterMap<Weight>::RouterMap(const TransportCatalogue& depot, const json::Document& doc) :
		vertexes_(), edge_mileage_(), vertex_list_(), router_(BuildGraph(*this, depot, doc))
	{

	}


	template <typename Weight>
	json::Dict ReportRoute(const TransportCatalogue& depot, const json::Document& doc, std::string_view from_stop, std::string_view to_stop) {
		json::Dict result;

		using namespace std;

		static RouterMap<Weight> router_map(depot, doc);

		auto router_info = router_map.router_.BuildRoute(router_map.vertexes_.at({ *depot.FindStop(from_stop), false }),
			router_map.vertexes_.at({ *depot.FindStop(to_stop), false }));
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

				if (vertex_from.on_route)
				{
					span.insert({ string(TYPE_TEXT), string(BUS_TEXT) });
					span.insert({ string(BUS__TEXT), edge.bus->name_ });
					span.insert({ string(TIME_TEXT), edge.mileage.time });
					span.insert({ string(SPAN_COUNT_TEXT), edge.mileage.span_count });
					spans_result.push_back(move(span));
					span.clear();

				}
				else //это посадка на автобус
				{
					span.insert({ string(TYPE_TEXT), string(WAIT_TEXT) });
					span.insert({ string(TIME_TEXT), edge.mileage.time });
					span.insert({ string(STOP_NAME_TEXT), vertex_from.stop.name_ });
					spans_result.push_back(move(span));
					span.clear();
				}

			}
			result.insert({ string(ITEMS_TEXT), move(spans_result) });
		}
		return result;
	}


} //namespace transport