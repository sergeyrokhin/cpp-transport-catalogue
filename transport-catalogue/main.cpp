#include "input_reader.h"
#include "stat_reader.h"

#include <fstream>

using namespace transport;


int main() {
	BusDepot depot;
//    std::ifstream in("tsC_case0_input.txt"); // окрываем файл для чтения
//    std::ifstream in("tsC_case1_input.txt"); // окрываем файл для чтения
    //  std::ifstream in("tsC_test2_input.txt"); // окрываем файл для чтения
    //if (in.is_open())
    //{
    //    Load(depot, in);
    //    ReportBusDepot(depot);
    //    Report(depot, in);
    //}
    //in.close();     // закрываем файл
    Load(depot, std::cin);
    Report(depot, std::cin);
    return 0;
}