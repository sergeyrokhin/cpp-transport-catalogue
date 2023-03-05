#pragma once
#include <string>

#include "geo.h"
#include "map_renderer.h"

/*
 * В этом файле вы можете разместить классы/структуры, которые являются частью предметной области (domain)
 * вашего приложения и не зависят от транспортного справочника. Например Автобусные маршруты и Остановки. 
 *
 * Их можно было бы разместить и в transport_catalogue.h, однако вынесение их в отдельный
 * заголовочный файл может оказаться полезным, когда дело дойдёт до визуализации карты маршрутов:
 * визуализатор карты (map_renderer) можно будет сделать независящим от транспортного справочника.
 *
 * Если структура вашего приложения не позволяет так сделать, просто оставьте этот файл пустым.
 *
 */

namespace transport {
	class TransportCatalogue;

	struct Stop {
		Stop(const std::string_view name) : name_(std::string(name)) {}
		std::string name_;
		geo::Coordinates geo_;
		bool operator==(const Stop& rh) { return name_ == rh.name_; }
	};

	struct Bus {
		Bus(const std::string_view name) : name_(std::string(name)) {}
		std::string name_;
		std::deque<Stop*> bus_route_;
		bool roundtrip_ = false;
		bool operator==(const Bus& rh) { return name_ == rh.name_; }
	};

	svg::Document BusDepotMap(const TransportCatalogue& depot, const renderer::MapRenderer& renderer);
	svg::Text GetTextTemplateBus(const renderer::MapRenderer& renderer, bool underlayer = false);

} // namespace transport