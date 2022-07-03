#include "json.h"
#include "json_reader.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"

#include <vector>


int main() {

    transport_catalogue::TransportCatalogue tc;
    renderer::MapRenderer mr;
    transport_router::TransportRouter tr(tc);

    json_reader::JsonReader input(std::cin, tc, mr, tr);

    input.GetInfo(std::cout);

}
