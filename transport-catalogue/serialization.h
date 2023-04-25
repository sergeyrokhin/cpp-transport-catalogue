#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <fstream>

namespace transport
{
	void DeserializeFrom(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer, const std::string& file_name);
}