#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include <transport_catalogue.pb.h>

#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "serialization.h"


using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    transport_catalogue::TransportCatalogue tc;
    renderer::MapRenderer mr;
    transport_router::TransportRouter tr(tc);
    json_reader::JsonReader input(tc, mr, tr);
    serialization::Serialization sr(tc, input, mr, tr);
    
    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv) {

        input.ReadInfo(std::cin);
        sr.SaveDataBase();

    } else if (mode == "process_requests"sv) {

        input.ReadRequest(std::cin);
        sr.LoadDataBase();
        input.GetResult(std::cout);

    } else {
        PrintUsage();
        return 1;
    }
}
