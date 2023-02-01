#pragma once
#include "transport_catalogue.h"

#include <iostream>

namespace transport {
	void Load(BusDepot& depot, std::istream& input);
    inline std::string_view LRstrip(std::string_view line);

    std::string_view Lstrip(std::string_view line);
    std::string_view Rstrip(std::string_view line);

    std::pair<std::string_view, std::string_view> Split(std::string_view line, char by);

    std::string_view Unquote(std::string_view value);
    inline std::string_view LRstrip(std::string_view line) {
        return Lstrip(Rstrip(line));
    }
}
