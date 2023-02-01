#pragma once
#include "transport_catalogue.h"

#include <iostream>


namespace transport {
	void Report(BusDepot& depot, std::istream& input);
	void ReportBusDepot(BusDepot& depot);
}
