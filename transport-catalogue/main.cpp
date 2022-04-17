#include "geo.h"
#include "input_reader.h"
#include "transport_catalogue.h"
#include "stat_reader.h"

#include <sstream>

int main()
{
    using namespace std::string_literals;

    transport_catalogue::TransportCatalogue trans_cat_;

    transport_catalogue::input_reader::ProcessInput(trans_cat_, std::cin);
    transport_catalogue::stat_reader::ProcessRequest(trans_cat_, std::cin);

    return 0;
}
