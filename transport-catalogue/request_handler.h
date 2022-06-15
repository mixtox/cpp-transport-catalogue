#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include <optional>

class RequestHandler {
public:

    RequestHandler(const transport_catalogue::TransportCatalogue& tc, const renderer::MapRenderer& renderer);

    svg::Document RenderMap() const;

    json::Dict GetRoute(const json::Node& request);
    json::Dict GetStop(const json::Node& request);
    json::Dict GetMap(const json::Node& request);

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;

};
