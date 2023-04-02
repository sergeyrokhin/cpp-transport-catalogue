#include "transport_router.h"
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

} //namespace transport