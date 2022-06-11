#include <sstream>
#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue &tc, const renderer::MapRenderer &renderer)
    : tc_(tc)
    , renderer_(renderer) {}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(tc_.GetAllRouteInfo());
}

json::Dict RequestHandler::GetRoute(const json::Node& request) {
    using namespace std::string_literals;
    json::Dict result;

    const auto bus_info = tc_.GetRouteInfo(request.AsMap().at("name"s).AsString());
    int id = request.AsMap().at("id"s).AsInt();

    result["request_id"s] = id;
    if (bus_info.has_value()) {
        result["curvature"s] = bus_info.value().curvature;
        result["route_length"s] = bus_info.value().route_length;
        result["stop_count"] = bus_info.value().stop_count;
        result["unique_stop_count"] = bus_info.value().unique_stop;
    } else {
        result["error_message"s] = "not found"s;
    }

    return result;
}

json::Dict RequestHandler::GetStop(const json::Node& request) {
    using namespace std::string_literals;
    json::Dict result;
    auto stop_info = tc_.GetBusesForStop(request.AsMap().at("name"s).AsString());

    int id = request.AsMap().at("id"s).AsInt();
    result["request_id"s] = id;
    if (stop_info) {
        std::set<std::string> buses;
        for (auto* bus : *stop_info) {
            buses.insert(bus->bus_number);
        }

        json::Array bus_array;
        for (auto& bus : buses) {
            bus_array.push_back(bus);
        }

        result["buses"s] = bus_array;

    } else {
        result["error_message"s] = "not found"s;
    }
    return result;
}

json::Dict RequestHandler::GetMap(const json::Node& request) {
    using namespace std::string_literals;
    json::Dict result;

    int id = request.AsMap().at("id"s).AsInt();

    std::stringstream stream;
    renderer_.GetSVG(tc_.GetAllRouteInfo()).Render(stream);

    result["request_id"s] = id;
    result["map"s] = json::Node(stream.str());
    return result;
}

