#pragma once
#include "transport_catalogue.h"

#include <utility>
#include <string>
#include <string_view>
#include <istream>
#include <iostream>

namespace transport_catalogue::input_reader

{
    void ProcessInput(TransportCatalogue&, std::istream&);
}
