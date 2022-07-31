#include <sstream>
#include "request_handler.h"

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue &tc, const renderer::MapRenderer &renderer,
                               const transport_router::TransportRouter &router)
    : tc_(tc)
    , renderer_(renderer)
    , tr_(router) {}

svg::Document RequestHandler::RenderMap() const {
    return renderer_.GetSVG(tc_.GetAllBusesInfo());
}

json::Dict RequestHandler::GetBus(const json::Node& request) {
    using namespace std::string_literals;

    const auto bus_info = tc_.GetBusInfo(request.AsDict().at("name"s).AsString());
    int id = request.AsDict().at("id"s).AsInt();

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
                .AsDict();
    } else {
        return json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
                .EndDict()
                .Build()
                .AsDict();
    }
}

json::Dict RequestHandler::GetStop(const json::Node& request) {
    using namespace std::string_literals;
    auto stop_info = tc_.GetStopInfo(request.AsDict().at("name"s).AsString());
    int id = request.AsDict().at("id"s).AsInt();

    if (stop_info) {
        std::set<std::string> buses;
        for (auto* bus : *stop_info) {
            buses.insert(bus->bus_name);
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
                .AsDict();
    } else {
        return json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
                .EndDict()
                .Build()
                .AsDict();
    }
}

json::Dict RequestHandler::GetMap(const json::Node& request) {
    using namespace std::string_literals;

    std::stringstream stream;
    renderer_.GetSVG(tc_.GetAllBusesInfo()).Render(stream);

    int id = request.AsDict().at("id"s).AsInt();

    return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("map"s)
            .Value(stream.str())
            .EndDict()
            .Build()
            .AsDict();
}

json::Dict RequestHandler::GetRoute(const json::Node& request) {
    using namespace std::string_literals;

    int id = request.AsDict().at("id"s).AsInt();

    std::string from = request.AsDict().at("from"s).AsString();
    std::string to = request.AsDict().at("to"s).AsString();
    const auto route_info = tr_.FindRoute(from, to);

    if (!route_info.has_value()) {
        return json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
                .EndDict()
                .Build()
                .AsDict();
    }
    json::Array items;
    double total_time = 0.0;
    for (auto* route_item : *route_info) {
        items.push_back(json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Wait"s)
                                .Key("stop_name"s)
                                .Value(route_item->from->name)
                                .Key("time"s)
                                .Value(route_item->travel_duration.waiting_time / 60)
                                .EndDict()
                                .Build()
                                .AsDict());
        total_time += route_item->travel_duration.waiting_time / 60;
        items.push_back(json::Builder{}
                                .StartDict()
                                .Key("type"s)
                                .Value("Bus"s)
                                .Key("bus"s)
                                .Value(route_item->route->bus_name)
                                .Key("span_count"s)
                                .Value(route_item->travel_duration.stops_number)
                                .Key("time"s)
                                .Value(route_item->travel_duration.travel_time / 60)
                                .EndDict()
                                .Build()
                                .AsDict());
        total_time += route_item->travel_duration.travel_time / 60;
    }
    return json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("total_time"s)
            .Value(total_time)
            .Key("items"s)
            .Value(items)
            .EndDict()
            .Build()
            .AsDict();


}
