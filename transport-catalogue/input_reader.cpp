#include "input_reader.h"
#include "transport_catalogue.h"

#include <string>

using namespace std;

namespace transport {

string_view Lstrip(string_view line) {
    while (!line.empty() && isspace(line[0])) {
        line.remove_prefix(1);
    }
    return line;
}
string_view Rstrip(string_view line) {
    auto end = line.size();
    while (!line.empty() && isspace(line[--end])) {
        line.remove_suffix(1);
    }
    return line;
}

pair<string_view, string_view> Split(string_view line, char by) {
    size_t pos = line.find(by);
    string_view left = LRstrip(line.substr(0, pos));

    if (pos < line.size() && pos + 1 < line.size()) {
        return { left, LRstrip(line.substr(pos + 1)) };
    }
    else {
        return { left, string_view() };
    }
}

string_view Unquote(string_view value) {
    if (!value.empty() && value.front() == '[') {
        value.remove_prefix(1);
    }
    if (!value.empty() && value.back() == ']') {
        value.remove_suffix(1);
    }
    return value;
}

}

/*
10
Stop Tolstopaltsevo : 55.611087, 37.208290
Stop Marushkino : 55.595884, 37.209755
Bus 256 : Biryulyovo Zapadnoye > Biryusinka > Universam > Biryulyovo Tovarnaya > Biryulyovo Passazhirskaya > Biryulyovo Zapadnoye
Bus 750: Tolstopaltsevo - Marushkino - Rasskazovka
Stop Rasskazovka : 55.632761, 37.333324
Stop Biryulyovo Zapadnoye : 55.574371, 37.651700
Stop Biryusinka : 55.581065, 37.648390
Stop Universam : 55.587655, 37.645687
Stop Biryulyovo Tovarnaya : 55.592028, 37.653656
Stop Biryulyovo Passazhirskaya : 55.580999, 37.659164
3
Bus 256
Bus 750
Bus 751
*/

using namespace std;

namespace transport {

	void Load(BusDepot& depot, std::istream& input) {


        string string_count;
        getline(input, string_count);
        int count = std::stoi(string_count);
        for (string line; count > 0; --count) {
            getline(input, line);
            auto [prefix, text] = Split(line, ' ');
            
            auto twice = Split(text, ':');
            auto name = LRstrip(twice.first);

            if (LRstrip(prefix) == "Bus")
            {
                Stop* stop_a = nullptr;
                Stop* stop_b = nullptr;
                auto& new_bus = depot.GetBus(name);
                bool not_finish = true; //для выхода из цикла
                bool is_reversible = false; // соединение остановок
                text = Rstrip(twice.second); //справа пробелы убрал, теперь подчищать только слева
                while (not_finish)
                {
                    auto not_reversible = text.find('>');
                    auto reversible = text.find('-');
                    auto comma = std::min(not_reversible, reversible); //какой знак ближе
                    if (comma > text.size()) { //значит это последний stop
                        name = Lstrip(text);
                        not_finish = false;
                    }
                    else {
                        name = LRstrip(text.substr(0, comma - 1));
                        text.remove_prefix(comma + 1);
                    }

                    stop_a = stop_b;
                    stop_b = &depot.GetStop(name);
                    if (stop_a != nullptr)
                    {
                        depot.AddStep(new_bus, stop_a, stop_b, is_reversible);
                    }
                    is_reversible = (reversible == comma); //для следующего интервала
                }
            }
            else { //command == "Stop"
                twice = Split(twice.second, ',');
                auto latitude = twice.first;
                twice = Split(twice.second, ',');
                auto& stop = depot.GetStop(name, std::stod(string(latitude)), std::stod(string(twice.first)));

				while (!twice.second.empty())
				{
					twice = Split(twice.second, ',');
					auto m_position = twice.first.find('m');
					auto distance = stol(string(twice.first.substr(0, m_position)));
                    auto stop_name = twice.first.substr(m_position + 5);
                    stop.distances_[&depot.GetStop(stop_name)] =  distance;
				}
			}
		}
	}
}