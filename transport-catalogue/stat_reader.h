#pragma once
#include "transport_catalogue.h"

#include <iostream>


namespace transport {
	void Report(BusDepot& depot, std::istream& input = std::cin, std::ostream& output = std::cout);
	void ReportBusDepot(BusDepot& depot, std::ostream& output = std::cout);
	std::ostream& operator<<(std::ostream& os, const Stop& stop);
	std::ostream& operator<<(std::ostream& os, const Bus& bus);
}
