#pragma once

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "graph.h"
#include "router.h"

#include <string_view>
#include <unordered_map>
#include <algorithm>

namespace transport {
	using Weight = double;

	using Graph = graph::DirectedWeightedGraph<Weight>;

	struct Mileage {
		double duration_boarding = 0;
		double duration = 0;
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
		RouterMap(const TransportCatalogue& catalogue) :
			catalogue_(catalogue), vertexes_(), edge_mileage_(), vertex_list_(), router_(BuildGraph(*this)) {
		}

		const TransportCatalogue& catalogue_;
		IndexStops vertexes_; //Поиск ID вершин
		EdgeID_EdgeMileage edge_mileage_; //подробная информация о ребрах по ID по поряюку
		std::vector<Vertex> vertex_list_; //список, по поряюку их VertexID
		graph::Router<Weight> router_;
	};

	struct StopBuildup {
		graph::VertexId vertex_id;
		Mileage mileage = {};
	};

	template <typename It>
	void FillGraphRoutes(Graph& graph, RouterMap<Weight>& router_map, const Bus& bus, const It first_stop, const It last_stop)
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
				auto [lenght, _] = DistCalculate(router_map.catalogue_, { past_stop, *stop_it }); // считаем расстояние от прошлой остановки
				auto router_time = lenght / router_map.catalogue_.GetBusVelocity();

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
			stop_buildup.push_back({ { vertex_stop_id }, {router_map.catalogue_.GetBusWaitTime(), router_map.catalogue_.GetBusWaitTime()} });
			past_stop = *stop_it; //поехали на следующую
		}
	}
	
	template <typename Weight>
	Graph& BuildGraph(RouterMap<Weight>& router_map) {

		//создание вершин графа
		auto& stops = router_map.catalogue_.GetAllStops();
		for (auto& stop : stops)
		{
			router_map.vertex_list_.push_back({stop});
			router_map.vertexes_.insert({ {stop}, router_map.vertexes_.size() });
		}
		static Graph graph(router_map.vertexes_.size()); //по количеству вершин

		//создание рёбер графа
		auto& buses = router_map.catalogue_.GetAllBuses();
		for (auto& bus : buses)
		{
			FillGraphRoutes(graph, router_map, bus, bus.bus_route_.begin(), bus.bus_route_.end());

			if (!bus.roundtrip_) //не круговой, будет двигаться в обратную сторону
			{
				FillGraphRoutes(graph, router_map, bus, bus.bus_route_.rbegin(), bus.bus_route_.rend());
			}
		}

		return graph;
	}

} //namespace transport