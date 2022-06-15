#include <sstream>
#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue& tc, const renderer::MapRenderer& renderer)
    : tc_(tc)
    , renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(tc_.GetAllRouteInfo());
}

json::Dict RequestHandler::GetRoute(const json::Node& request) {
    using namespace std::string_literals;

    const auto bus_info = tc_.GetRouteInfo(request.AsMap().at("name"s).AsString());
    int id = request.AsMap().at("id"s).AsInt();

    if (bus_info.has_value()) {
        return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("curvature"s)
            .Value(bus_info.value().curvature)
            .Key("route_length"s)
            .Value(bus_info.value().route_length)
            .Key("stop_count"s)
            .Value(bus_info.value().stop_count)
            .Key("unique_stop_count"s)
            .Value(bus_info.value().unique_stop)
            .EndDict()
            .Build()
            .AsMap();
    }
    else {
        return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("error_message"s)
            .Value("not found"s)
            .EndDict()
            .Build()
            .AsMap();
    }
}

json::Dict RequestHandler::GetStop(const json::Node& request) {
    using namespace std::string_literals;
    auto stop_info = tc_.GetBusesForStop(request.AsMap().at("name"s).AsString());
    int id = request.AsMap().at("id"s).AsInt();

    if (stop_info) {
        std::set<std::string> buses;
        for (auto* bus : *stop_info) {
            buses.insert(bus->bus_number);
        }

        json::Array bus_array;
        for (auto& bus : buses) {
            bus_array.push_back(bus);
        }

        return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("buses"s)
            .Value(bus_array)
            .EndDict()
            .Build()
            .AsMap();
    }
    else {
        return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("error_message"s)
            .Value("not found"s)
            .EndDict()
            .Build()
            .AsMap();
    }
}

json::Dict RequestHandler::GetMap(const json::Node& request) {
    using namespace std::string_literals;

    std::stringstream stream;
    renderer_.GetSVG(tc_.GetAllRouteInfo()).Render(stream);

    int id = request.AsMap().at("id"s).AsInt();

    return json::Builder{}
        .StartDict()
        .Key("request_id"s)
        .Value(id)
        .Key("map"s)
        .Value(stream.str())
        .EndDict()
        .Build()
        .AsMap();
}

