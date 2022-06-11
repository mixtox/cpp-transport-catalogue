#include "json_reader.h"
#include "transport_catalogue.h"
#include "json.h"
#include "request_handler.h"
#include "map_renderer.h"

#include <fstream>
#include <vector>


int main() {
    transport_catalogue::TransportCatalogue trans_cat_;
    renderer::MapRenderer mr;

    json_reader::JsonReader input(std::cin, trans_cat_, mr);
    input.GetInfo(std::cout);

}
