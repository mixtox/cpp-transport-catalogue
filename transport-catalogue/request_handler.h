#pragma once

#include "transport_catalogue.h"
#include "transport_router.h"
#include "map_renderer.h"
#include "json.h"
#include "json_builder.h"
#include <optional>

class RequestHandler {
public:

    RequestHandler(const transport_catalogue::TransportCatalogue &tc, const renderer::MapRenderer &renderer,
                   const transport_router::TransportRouter& router);

    svg::Document RenderMap() const;

    json::Dict GetBus(const json::Node& request);
    json::Dict GetStop(const json::Node& request);
    json::Dict GetMap(const json::Node& request);
    json::Dict GetRoute(const json::Node& request);

private:
    const transport_catalogue::TransportCatalogue& tc_;
    const renderer::MapRenderer& renderer_;
    const transport_router::TransportRouter& tr_;
};
