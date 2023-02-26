#include <fstream>

#include "json_reader.h"

using namespace std;

//#define DEBUG

int main() {
    setlocale(LC_ALL, ".UTF8");
    setlocale(LC_NUMERIC, "us_US.UTF-8");

    transport::TransportCatalogue depot;

#ifdef DEBUG
    ifstream in("s10_final_opentest_1.json");
    if (!(in))
    {
        cout << "Error open file\n"s;
        return 0;
    }

    ofstream out("output.json");
    if (!(out))
    {
        cout << "Error open file\n"s;
        return 0;
    }

#else
    istream& in = cin;
    ostream& out = cout;
#endif // DEBUG

    json::Document creature = json::Load(in);
    transport::Load(depot, creature);
    transport::Report(depot, creature, out);

#ifdef DEBUG
//    transport::ReportBusDepot(depot, cout);
#endif // DEBUG

    return 0;
}
