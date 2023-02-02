#include "input_reader.h"
#include "stat_reader.h"

#include <fstream>

using namespace transport;

//#define DEBUG

int main() {
	BusDepot depot;
#ifdef DEBUG
    std::ifstream in("tsC_case0_input.txt");
    //  std::ifstream in("tsC_case1_input.txt");
    //  std::ifstream in("tsC_test2_input.txt");
    if (in.is_open())
    {
        Load(depot, in);
        ReportBusDepot(depot, std::cout);
        Report(depot, in, std::cout);
    }
    in.close();
#else
    Load(depot);
    Report(depot);
#endif // DEBUG
    return 0;
}