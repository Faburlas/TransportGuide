#include <fstream>
#include <cassert>
#include <sstream>

#include "json_reader.h"
#include "transport_catalogue.h"
#include "json.h"

int main() {
    using namespace json;
    using namespace std::literals;

    tg::TransportGuide tg1;
    JsonReader reader(tg1);

    reader.RunCommands(std::cin,std::cout);
 
}