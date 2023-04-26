#include "ranges.h"
#include "router.h"
#include "json_reader.h"

//#include <string_view>

using namespace std;

namespace transport {

	void AddEdgeInGraph(Graph& graph, EdgeID_EdgeMileage& id_edge_mileage, const EdgeMileage& edgemileage) {
		graph::EdgeId edge_id = graph.AddEdge({ edgemileage.edge.from, edgemileage.edge.to, edgemileage.mileage.duration });
		if (id_edge_mileage.size() < edge_id + 1)
		{
			id_edge_mileage.resize(edge_id + 1);
		}
		id_edge_mileage[edge_id] = edgemileage;
	}

    static constexpr string_view ROUTER_SETTINGS_TEXT = "routing_settings"sv;
    static constexpr string_view BUS_VELOCITY_TEXT = "bus_velocity"sv;
    static constexpr string_view BUS_WAIT_TIME_TEXT = "bus_wait_time"sv;
    static constexpr double KM_H_TO_METR_MIN_CONVERTER = 1000. / 60;

    RouterSetting::RouterSetting(const RouterSetting& other) : bus_wait_time_(other.bus_wait_time_), bus_velocity_(other.bus_velocity_)
    {
    }

    RouterSetting::RouterSetting(const json::Document& doc)
	{
        auto& root = doc.GetRoot().AsDict();

        if (root.count(ROUTER_SETTINGS_TEXT)) {
            auto& property_router = (*root.find(ROUTER_SETTINGS_TEXT)).second.AsDict();
            if (property_router.count(BUS_WAIT_TIME_TEXT)) bus_wait_time_ = (*property_router.find(BUS_WAIT_TIME_TEXT)).second.AsInt();
            if (property_router.count(BUS_VELOCITY_TEXT))  bus_velocity_ = (*property_router.find(BUS_VELOCITY_TEXT)).second.AsDouble()
                                                                                * KM_H_TO_METR_MIN_CONVERTER;
        }
    }

} //namespace transport