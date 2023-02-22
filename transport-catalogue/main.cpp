#include "json_reader.h"

#include <fstream>

//using namespace transport;

using namespace std;

//#define DEBUG

int main() {
    setlocale(LC_ALL, ".UTF8");
    std::setlocale(LC_NUMERIC, "us_US.UTF-8");

    transport::BusDepot depot;

#ifdef DEBUG
    std::ifstream in("input.json");
    if ( ! (in).is_open())
    {
        cout << "Error open file"s << std::endl;
        return 0;
    }

    std::ofstream out("output.svg");
    if (!(out).is_open())
    {
        cout << "Error open file"s << std::endl;
        return 0;
    }

#else
    std::istream& in = std::cin;
    std::ostream& out = std::cout;
#endif // DEBUG

    json::Document creature = json::Load(in);
    transport::Load(depot, creature);
    transport::Report(depot, creature, out);

#ifdef DEBUG
//    transport::ReportBusDepot(depot, std::cout);
    if (in.is_open()) in.close();
    if (out.is_open()) out.close();
#endif // DEBUG

    return 0;
}
