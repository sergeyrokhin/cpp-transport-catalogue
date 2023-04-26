#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"

#include <fstream>

namespace transport
{
	void DeserializeFrom(TransportCatalogue& catalogue, renderer::MapRenderer& map_renderer, RouterSetting& router_settings, const std::string& file_name);
}