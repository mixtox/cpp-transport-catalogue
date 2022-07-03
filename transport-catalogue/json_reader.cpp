#include <sstream>
#include "json_reader.h"

namespace json_reader {

    JsonReader::JsonReader(std::istream &input, transport_catalogue::TransportCatalogue &catalogue,
                           renderer::MapRenderer &renderer, transport_router::TransportRouter &router)
            :input_(json::Load(input)), tc_(catalogue), mr_(renderer), tr_(router) {
        ParseStop();
        ParseBus();

    }

    void JsonReader::ParseStop() {
        using namespace std::string_literals;
        auto requests = input_.GetRoot().AsDict().at("base_requests"s).AsArray();

        for (const auto& request : requests) {
            if (request.AsDict().at("type"s) == "Stop"s) {
                const auto& request_map = request.AsDict();
                const auto& stop_name = request_map.at("name"s).AsString();
                const auto& latitude = request_map.at("latitude"s).AsDouble();
                const auto& longitude = request_map.at("longitude"s).AsDouble();
                tc_.AddStop(stop_name, {latitude, longitude});
            }
        }

        for (const auto& request : requests) {
            if (request.AsDict().at("type"s) == "Stop"s) {
                const auto& request_map = request.AsDict();
                const auto& first_stop = request_map.at("name"s).AsString();
                const auto& stop_distance = request_map.at("road_distances"s).AsDict();
                for (auto [second_stop, real_distance] : stop_distance) {
                    tc_.AddDistance(tc_.FindStop(std::string(first_stop)), tc_.FindStop(second_stop), real_distance.AsDouble());
                }
            }
        }
    }

    void JsonReader::ParseBus() {
        using namespace std::string_literals;
        auto requests = input_.GetRoot().AsDict().at("base_requests"s).AsArray();
        for (const auto& request : requests) {
            if (request.AsDict().at("type"s) == "Bus"s) {
                const auto& request_map = request.AsDict();
                const auto& bus_name = request_map.at("name"s).AsString();
                const auto& stops_name = request_map.at("stops"s).AsArray();
                std::vector<std::string> stops_in_tc;
                for_each(
                        stops_name.begin(), stops_name.end(),
                        [&stops_in_tc](const auto stop){
                            stops_in_tc.push_back(stop.AsString());
                        }
                        );
                tc_.AddBus(bus_name, stops_in_tc, request_map.at("is_roundtrip"s).AsBool());
            }
        }
    }

    void JsonReader::GetInfo(std::ostream &out) {
        using namespace std::string_literals;
        RequestHandler handler(tc_, mr_, tr_);

        bool tr_first_init = true;

        json::Array result;
        auto requests = input_.GetRoot().AsDict().at("stat_requests"s).AsArray();
        
        for (const auto request : requests) {
            std::string type = request.AsDict().at("type"s).AsString();
            if (type == "Bus"s) {
                result.push_back(std::move(handler.GetBus(request)));
            }
            if (type == "Stop"s) {
                result.push_back(std::move(handler.GetStop(request)));
            }
            if (type == "Map"s) {
                renderer::RenderSettings renderSettings = ReadRenderSettings();
                mr_.SetRendererSettings(renderSettings);
                result.push_back(std::move(handler.GetMap(request)));
            }
            if (type == "Route"s) {
                if (tr_first_init) {
                    domain::RouteSettings route_settings = ReadRouteSettings();
                    tr_.RouteInit(route_settings);
                    tr_first_init = false;
                }
                result.push_back(std::move(handler.GetRoute(request)));
            }
        }
        
        json::Document doc(std::move(result));
        json::Print(doc, out);
    }

    renderer::RenderSettings JsonReader::ReadRenderSettings() {
        using namespace std::string_literals;
        renderer::RenderSettings render_settings;
        
        auto read_settings = input_.GetRoot().AsDict().at("render_settings"s).AsDict();
        render_settings.width = read_settings.at("width"s).AsDouble();
        render_settings.height = read_settings.at("height"s).AsDouble();
        render_settings.padding = read_settings.at("padding"s).AsDouble();
        render_settings.line_width = read_settings.at("line_width"s).AsDouble();
        render_settings.stop_radius = read_settings.at("stop_radius"s).AsDouble();
        render_settings.bus_label_font_size = read_settings.at("bus_label_font_size"s).AsInt();
        render_settings.bus_label_offset = svg::Point(read_settings.at("bus_label_offset"s).AsArray()[0].AsDouble(),
                                                      read_settings.at("bus_label_offset"s).AsArray()[1].AsDouble());
        render_settings.stop_label_font_size = read_settings.at("stop_label_font_size"s).AsInt();
        render_settings.stop_label_offset = svg::Point(read_settings.at("stop_label_offset"s).AsArray()[0].AsDouble(),
                                                       read_settings.at("stop_label_offset"s).AsArray()[1].AsDouble());
        
        if (read_settings.at("underlayer_color"s).IsString()) {
            render_settings.underlayer_color = read_settings.at("underlayer_color"s).AsString();
        } else if (read_settings.at("underlayer_color"s).AsArray().size() == 3) {
            render_settings.underlayer_color = svg::Rgb(read_settings.at("underlayer_color"s).AsArray()[0].AsInt(),
                                                        read_settings.at("underlayer_color"s).AsArray()[1].AsInt(),
                                                        read_settings.at("underlayer_color"s).AsArray()[2].AsInt());
        } else {
            render_settings.underlayer_color = svg::Rgba(read_settings.at("underlayer_color"s).AsArray()[0].AsInt(),
                                                         read_settings.at("underlayer_color"s).AsArray()[1].AsInt(),
                                                         read_settings.at("underlayer_color"s).AsArray()[2].AsInt(),
                                                         read_settings.at("underlayer_color"s).AsArray()[3].AsDouble());
        }
        render_settings.underlayer_width = read_settings.at("underlayer_width"s).AsDouble();
        
        for (const auto& color : read_settings.at("color_palette"s).AsArray()) {
            if (color.IsString()) {
                render_settings.color_palette.push_back(color.AsString());
            } else if (color.AsArray().size() == 3) {
                render_settings.color_palette.push_back(svg::Rgb(color.AsArray()[0].AsInt(),
                                                                 color.AsArray()[1].AsInt(),
                                                                 color.AsArray()[2].AsInt()));
            } else {
                render_settings.color_palette.push_back(svg::Rgba(color.AsArray()[0].AsInt(),
                                                                  color.AsArray()[1].AsInt(),
                                                                  color.AsArray()[2].AsInt(),
                                                                  color.AsArray()[3].AsDouble()));
            }
        }
        
        return render_settings;
    }

    domain::RouteSettings JsonReader::ReadRouteSettings() {
        using namespace std::string_literals;
        domain::RouteSettings setting;
        json::Dict route_settings = input_.GetRoot().AsDict().at("routing_settings"s).AsDict();
        setting.bus_wait_time = route_settings.at("bus_wait_time"s).AsInt();
        setting.bus_velocity = route_settings.at("bus_velocity"s).AsDouble();
        return setting;
    }


}
