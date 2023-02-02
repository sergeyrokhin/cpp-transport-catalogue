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

using namespace std;

namespace transport {

    void LoadBus(BusDepot& depot, string_view name, string_view text) {
        Stop* stop_a = nullptr;
        Stop* stop_b = nullptr;
        auto& new_bus = depot.GetBus(name);
        bool not_finish = true; //для выхода из цикла
        bool is_reversible = false; // соединение остановок
        text = Rstrip(text); //справа пробелы убрал, теперь подчищать только слева
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
                depot.AddStep(new_bus, { stop_a, stop_b }, is_reversible);
            }
            is_reversible = (reversible == comma); //для следующего интервала
        }
    }

    void LoadStop(BusDepot& depot, string_view name, string_view text) {
        auto twice = Split(text, ',');
        auto latitude = twice.first;
        twice = Split(twice.second, ',');
        auto& stop = depot.GetStop(name, std::stod(string(latitude)), std::stod(string(twice.first)));

        while (!twice.second.empty())
        {
            twice = Split(twice.second, ',');
            auto m_position = twice.first.find('m');
            auto distance = stol(string(twice.first.substr(0, m_position)));
            auto stop_name = twice.first.substr(m_position + 5); //неприятная 5-ка, но обещали, что шаблон жесткий 
            depot.AddDistance({ &stop, &depot.GetStop(stop_name) }, distance);
        }
    }

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
                LoadBus(depot, name, twice.second);
            }
            else { //command == "Stop"
                LoadStop(depot, name, twice.second);
            }
		}
	}
}