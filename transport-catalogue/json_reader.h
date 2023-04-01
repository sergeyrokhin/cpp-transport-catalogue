#pragma once
#include "json.h"
#include "transport_catalogue.h"
/*
 * Здесь можно разместить код наполнения транспортного справочника данными из JSON,
 * а также код обработки запросов к базе и формирование массива ответов в формате JSON
 */

namespace transport {
	void Load(TransportCatalogue& depot, const json::Document& doc);
	void Report(const TransportCatalogue& depot, const json::Document& doc, std::ostream& output = std::cout);
	//расстояние с учетом справочной информации
	//расстояние географическое

	void ReportBusDepot(TransportCatalogue& depot, std::ostream& output = std::cout);
	std::ostream& operator<<(std::ostream& os, const Stop& stop);
	std::ostream& operator<<(std::ostream& os, const Bus& bus);

	using namespace std::literals;
	static constexpr std::string_view ERROR_TEXT = "error_message"sv;
	static constexpr std::string_view NOT_FOUND_TEXT = "not found"sv;
	static constexpr std::string_view TYPE_TEXT = "type"sv;
	static constexpr std::string_view BUS_TEXT = "Bus"sv;
}
