#pragma once
#include "transport_catalogue.h"
#include "input_reader.h"

#include <utility>
#include <string>
#include <string_view>
#include <istream>
#include <iostream>

namespace transport_catalogue::stat_reader

{
    std::ostream& operator<<(std::ostream& os, const Stop& stop);
    std::ostream& operator<<(std::ostream& os, const Bus* bus);

    std::ostream& ProcessRequests(TransportCatalogue&, std::istream&, std::ostream&);
    std::ostream& ExecuteRequest(TransportCatalogue&, RequestQuery&, std::ostream&);
}
