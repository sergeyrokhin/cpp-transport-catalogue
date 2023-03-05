#include "json_builder.h"
#include <iostream>
#include <fstream>

#include "json_reader.h"

using namespace std;

//#define DEBUG

int main() {


    transport::TransportCatalogue depot;

#ifdef DEBUG

    setlocale(LC_ALL, ".UTF8");
    setlocale(LC_NUMERIC, "us_US.UTF-8");


    json::Print(
        json::Document{
            json::Builder{}
            .StartDict()
                .Key("key1"s).Value(123)
                .Key("key2"s).Value("value2"s)
                .Key("key3"s).StartArray()
                    .Value(456)
                    .StartDict().EndDict()
                    .StartDict()
                        .Key(""s)
                        .Value(nullptr)
                    .EndDict()
                    .Value(""s)
                .EndArray()
            .EndDict()
            .Build()
        },
        cout
    );
    cout << endl;

    json::Print(
        json::Document{
            json::Builder{}
            .Value("just a string"s)
            .Build()
        },
        cout
    );
    cout << endl;

    ifstream in("input.json");
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
