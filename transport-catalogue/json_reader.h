#pragma once

#include "domain.h"
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include "svg.h"
#include <string>
#include <string_view>
#include <vector>


namespace json_reader {

    class JsonReader {
    public:

        JsonReader(std::istream &input, transport_catalogue::TransportCatalogue &catalogue,
                   renderer::MapRenderer &renderer, transport_router::TransportRouter &router);

        void GetInfo (std::ostream &out);

    private:

        json::Document input_;
        transport_catalogue::TransportCatalogue& tc_;
        renderer::MapRenderer& mr_;
        transport_router::TransportRouter& tr_;

        void ParseStop ();
        void ParseBus ();

        renderer::RenderSettings ReadRenderSettings ();
        domain::RouteSettings ReadRouteSettings ();

    };
}
