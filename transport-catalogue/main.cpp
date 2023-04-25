#include "json_builder.h"
#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;
using namespace std;

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
    string filename = file_name + ".json";
    if (!(out))
    {
        cout << "Error open output file: " << filename << " \n"s;
    }
    return out;
}


void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

void MakeBase(istream& in, ostream& out) {
    transport::TransportCatalogue catalogue;
    json::Document creature = json::Load(in);
    renderer::MapRenderer map_renderer(creature);
    transport::Load(catalogue, creature);
    transport::SaveTo(catalogue, map_renderer, creature);
    transport::Report(catalogue, map_renderer, creature, out);
}

void ProcessRequests(istream& in, ostream& out) {
    transport::TransportCatalogue catalogue;
    json::Document creature = json::Load(in);
    renderer::MapRenderer map_renderer;
    transport::LoadFrom(catalogue, map_renderer, creature);
    transport::Report(catalogue, map_renderer, creature, out);
}


int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);


////////
//#define DEBUG

#ifdef DEBUG

    setlocale(LC_ALL, ".UTF8");
    setlocale(LC_NUMERIC, "us_US.UTF-8");
    //const string file_name = "process_requests"s;
    const string file_name(mode);

    ifstream in = OpenInputFile(file_name + ".json");
    if (!(in)) return 0;

    ofstream out = OpenOutputFile(file_name + "_out.json");
    if (!(out)) return 0;

#else
    istream& in = cin;
    ostream& out = cout;

#endif // DEBUG

    if (mode == "make_base"sv) {

        MakeBase(in, out);

    } else if (mode == "process_requests"sv) {

        ProcessRequests(in, out);

    } else {
        PrintUsage();
        return 1;
    }
}