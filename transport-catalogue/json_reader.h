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
#include <filesystem>


namespace json_reader {
    class JsonReader {
    public:
        JsonReader(transport_catalogue::TransportCatalogue &catalogue,
                   renderer::MapRenderer &renderer, transport_router::TransportRouter &router);

        void ReadInfo(std::istream &input);
        void ReadRequest(std::istream &input);

        void GetResult(std::ostream &out);
        const std::string GetSerializationSetting() const;

    private:
        std::string file_names_;
        json::Document input_ = json::Document(json::Node());

        transport_catalogue::TransportCatalogue &tc_;
        renderer::MapRenderer &mr_;
        transport_router::TransportRouter &tr_;

        void ParseStop();
        void ParseBus();

        renderer::RenderSettings ReadRenderSettings();
        domain::RouteSettings ReadRouteSettings();

        void ReadSerializationSettings();
    };
}

