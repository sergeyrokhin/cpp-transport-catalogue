#include "json_builder.h"
#include <iostream>
#include <fstream>

#include "json_reader.h"

using namespace std;

//#define DEBUG

void TestJsonBuilding() {
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
}


ifstream OpenInputFile(const string& file_name) {
    ifstream in(file_name);
    string filename = file_name;
    if (!(in))
    {
        cout << "Error open input file: " << filename << " \n"s;
    }
    return in;
}

ofstream OpenOutputFile(const string& file_name) {
    ofstream out(file_name);
    // "\\json\\" +
    string filename =  file_name + ".json";
    if (!(out))
    {
        cout << "Error open output file: " << filename << " \n"s;
    }
    return out;
}


int main() {


    transport::TransportCatalogue catalogue;

#ifdef DEBUG

    setlocale(LC_ALL, ".UTF8");
    setlocale(LC_NUMERIC, "us_US.UTF-8");

    const string file_name = "input4"s;

    ifstream in = OpenInputFile(file_name + ".json");
    if (!(in)) return 0;

    ofstream out = OpenOutputFile(file_name + "_out.json");
    if (!(out)) return 0;

    json::Document creature = json::Load(in);
        transport::Load(catalogue, creature);
        transport::Report(catalogue, creature, out);
    if(false){
        ofstream out = OpenOutputFile(file_name + ".svg");
        if (!(out)) return 0;

        auto map_doc = transport::CatalogueMap(catalogue, creature);
        map_doc.MapRender(out);
    }
    //transport::ReportBuscatalogue(catalogue, cout);


#else
    istream& in = cin;
    ostream& out = cout;

    json::Document creature = json::Load(in);
    transport::Load(catalogue, creature);
    transport::Report(catalogue, creature, out);
#endif // DEBUG

#ifdef DEBUG
    //    transport::ReportBuscatalogue(catalogue, cout);
#endif // DEBUG

    return 0;
}
