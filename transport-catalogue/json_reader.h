#pragma once
#include "json.h"
#include "transport_catalogue.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport {
	void Load(BusDepot& depot, const json::Document& doc);
	void Report(BusDepot& depot, const json::Document& doc, std::ostream& output = std::cout);
	//расстояние с учетом справочной информации
	//расстояние географическое

	void ReportBusDepot(BusDepot& depot, std::ostream& output = std::cout);
	std::ostream& operator<<(std::ostream& os, const Stop& stop);
	std::ostream& operator<<(std::ostream& os, const Bus& bus);
}
