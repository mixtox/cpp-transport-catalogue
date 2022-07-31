#include "serialization.h"

namespace serialization {

    Serialization::Serialization(transport_catalogue::TransportCatalogue &catalogue,
                                 const json_reader::JsonReader &input,
                                 renderer::MapRenderer &renderer,
                                 transport_router::TransportRouter &router)
            : tc_(catalogue), jr_(input), mr_(renderer), tr_(router) {
    }

//Transport catalogue
//Save methods
    tc_serialize::Stop Serialization::SaveStop(const domain::Stop &stop) const {
        tc_serialize::Stop stop_base;

        stop_base.set_stop_name(stop.name);
        stop_base.mutable_coordinates()->set_latitude(stop.coordinates.lat);
        stop_base.mutable_coordinates()->set_longitude(stop.coordinates.lng);

        return stop_base;
    }

    tc_serialize::Bus Serialization::SaveBus(const domain::Bus &bus) const {
        tc_serialize::Bus bus_base;

        bus_base.set_bus_name(bus.bus_name);
        for (const auto stop: bus.bus_stops) {
            bus_base.add_stop_in_bus_route(stop->name);
        }
        bus_base.set_is_circle(bus.is_circle);

        return bus_base;
    }

    tc_serialize::DistanceBetweenStop
    Serialization::SaveDistance(domain::StopPtr from, domain::StopPtr to, double distance) const {
        tc_serialize::DistanceBetweenStop stop_distance;

        *stop_distance.mutable_stop_from_name() = from->name;
        *stop_distance.mutable_stop_to_name() = to->name;
        stop_distance.set_distance(distance);

        return stop_distance;
    }

    void Serialization::SaveStops() {
        for (const auto stop: tc_.GetStopBase()) {
            *data_base_.mutable_transport_cat_core()->add_stops() = std::move(SaveStop(stop));
        }
    }

    void Serialization::SaveBuses() {
        for (const auto bus: tc_.GetBusBase()) {
            *data_base_.mutable_transport_cat_core()->add_buses() = std::move(SaveBus(bus));
        }
    }

    void Serialization::SaveDistance() {
        for (const auto [stop_pair, distance]: tc_.GetDistanceBase()) {
            *data_base_.mutable_transport_cat_core()->add_distance_between_stop() = std::move(
                    SaveDistance(stop_pair.first, stop_pair.second, distance));
        }
    }

    void Serialization::SaveDataBase() {
        auto file_name = jr_.GetSerializationSetting();

        std::fstream output(file_name, std::ios::out | std::ios::trunc | std::ios::binary);

        SaveStops();
        SaveBuses();
        SaveDistance();
        SaveRenderSettings();
        SaveRouteSetting();
        SaveGraphs();

        data_base_.SerializeToOstream(&output);

    }

//Load methods
    void Serialization::LoadDataBase() {
        auto file_name = jr_.GetSerializationSetting();
        std::fstream input(file_name, std::ios::in | std::ios::binary);
        data_base_.ParseFromIstream(&input);

        LoadStops();
        LoadBuses();
        LoadDistance();
        LoadRenderSettings();
        LoadRouteSetting();
        LoadGraphs();
    }

    void Serialization::LoadStops() {
        for (const auto stop : data_base_.transport_cat_core().stops()) {
            LoadStop(stop);
        }
    }

    void Serialization::LoadBuses() {
        for (int i = 0; i < data_base_.transport_cat_core().buses_size(); ++i) {
            LoadBus(data_base_.mutable_transport_cat_core()->buses(i));
        }
    }

    void Serialization::LoadDistance() {
        for (int i = 0; i < data_base_.transport_cat_core().distance_between_stop_size(); ++i) {
            LoadDistance(data_base_.transport_cat_core().distance_between_stop(i));
        }
    }

    void Serialization::LoadStop(const tc_serialize::Stop &stop) {
        domain::Stop result;

        result.name = stop.stop_name();
        result.coordinates.lat = stop.coordinates().latitude();
        result.coordinates.lng = stop.coordinates().longitude();

        tc_.AddStop(result.name, result.coordinates);
    }

    void Serialization::LoadBus(const tc_serialize::Bus &bus) {
        std::vector<std::string> stop_name;

        for (int i = 0; i < bus.stop_in_bus_route_size(); ++i) {
            stop_name.push_back(bus.stop_in_bus_route(i));
        }
        tc_.AddBus(bus.bus_name(), stop_name, bus.is_circle());
    }

    void Serialization::LoadDistance(const tc_serialize::DistanceBetweenStop &distance) {
        tc_.SetDistanceStops(tc_.FindStop(distance.stop_from_name()), tc_.FindStop(distance.stop_to_name()),
                             distance.distance());
    }

//Map renderer
//Save method
    void Serialization::SaveRenderSettings() {
        renderer::RenderSettings settings = mr_.GetRenderSetting();
        map_proto::RenderSettings result;

        result.set_width(settings.width);
        result.set_height(settings.height);

        result.set_padding(settings.padding);
        result.set_line_width(settings.line_width);
        result.set_stop_radius(settings.stop_radius);

        result.set_bus_label_font_size(settings.bus_label_font_size);
        *result.mutable_bus_label_offset() = SavePoint(settings.bus_label_offset);

        result.set_stop_label_font_size(settings.stop_label_font_size);
        *result.mutable_stop_label_offset() = SavePoint(settings.stop_label_offset);

        *result.mutable_underlayer_color() = SaveColor(settings.underlayer_color);
        result.set_underlayer_width(settings.underlayer_width);

        for (const auto color: settings.color_palette) {
            *result.add_color_palette() = SaveColor(color);
        }

        *data_base_.mutable_map_core()->mutable_render_setting() = std::move(result);
    }

    svg_proto::Point Serialization::SavePoint(const svg::Point &point) {
        svg_proto::Point result;

        result.set_y(point.y);
        result.set_x(point.x);

        return result;
    }

    svg_proto::Color Serialization::SaveColor(const svg::Color &color) {
        svg_proto::Color result;
        switch (color.index()) {
            case 1: {
                *result.mutable_string_color() = std::get<1>(color);
                break;
            }
            case 2: {
                *result.mutable_rgb_color() = SaveRGB(std::get<2>(color));
                break;
            }
            case 3: {
                *result.mutable_rgba_color() = SaveRGBa(std::get<3>(color));
            }
        }
        return result;
    }

    svg_proto::RGB Serialization::SaveRGB(const svg::Rgb &rgb) {
        svg_proto::RGB result;

        result.set_red(rgb.red);
        result.set_green(rgb.green);
        result.set_blue(rgb.blue);

        return result;
    }

    svg_proto::RGBA Serialization::SaveRGBa(const svg::Rgba &rgba) {
        svg_proto::RGBA result;

        result.set_red(rgba.red);
        result.set_green(rgba.green);
        result.set_blue(rgba.blue);
        result.set_opacity(rgba.opacity);

        return result;
    }

//Load method
    void Serialization::LoadRenderSettings() {
        renderer::RenderSettings setting;

        setting.width = data_base_.map_core().render_setting().width();
        setting.height = data_base_.map_core().render_setting().height();

        setting.padding = data_base_.map_core().render_setting().padding();
        setting.line_width = data_base_.map_core().render_setting().line_width();
        setting.stop_radius = data_base_.map_core().render_setting().stop_radius();

        setting.bus_label_font_size = data_base_.map_core().render_setting().bus_label_font_size();
        setting.bus_label_offset = LoadPoint(data_base_.map_core().render_setting().bus_label_offset());

        setting.stop_label_font_size = data_base_.map_core().render_setting().stop_label_font_size();
        setting.stop_label_offset = LoadPoint(data_base_.map_core().render_setting().stop_label_offset());

        setting.underlayer_color = LoadColor(data_base_.map_core().render_setting().underlayer_color());
        setting.underlayer_width = data_base_.map_core().render_setting().underlayer_width();

        for (int i = 0; i < data_base_.map_core().render_setting().color_palette_size(); ++i) {
            setting.color_palette.push_back(LoadColor(data_base_.map_core().render_setting().color_palette(i)));
        }

        mr_.SetRendererSettings(setting);
    }

    svg::Point Serialization::LoadPoint(const svg_proto::Point &point) {
        svg::Point result;

        result.y = point.y();
        result.x = point.x();

        return result;
    }

    svg::Color Serialization::LoadColor(const svg_proto::Color &color) {
        if (color.has_rgb_color()) {
            return LoadRGB(color.rgb_color());
        } else if (color.has_rgba_color()) {
            return LoadRGBA(color.rgba_color());
        } else {
            return color.string_color();
        }
    }

    svg::Rgb Serialization::LoadRGB(const svg_proto::RGB &rgb) {
        return svg::Rgb{static_cast<uint8_t>(rgb.red()),
                        static_cast<uint8_t>(rgb.green()),
                        static_cast<uint8_t>(rgb.blue())};
    }

    svg::Rgba Serialization::LoadRGBA(const svg_proto::RGBA &rgba) {
        return svg::Rgba{static_cast<uint8_t>(rgba.red()),
                         static_cast<uint8_t>(rgba.green()),
                         static_cast<uint8_t>(rgba.blue()),
                         static_cast<double>(rgba.opacity())};

    }

//Transport router
//Save method
    void Serialization::SaveRouteSetting() {
        domain::RouteSettings settings = tr_.GetRouteSettings();

        data_base_.mutable_router_core()->mutable_route_setting()->set_bus_velocity(settings.bus_velocity);
        data_base_.mutable_router_core()->mutable_route_setting()->set_bus_wait_time(settings.bus_wait_time);
    }

    void Serialization::SaveGraphs() {
        SaveGraphVertex();
        SaveGraphEdge();
        SaveGraph();
    }

    void Serialization::SaveGraphVertex() {
        auto vertex = tr_.GetGraphVertex();
        for (const auto [stop, vertex_id]: vertex) {
            router_proto::Graph_Vertex_ID vertex_id_point;

            *vertex_id_point.mutable_stop_name() = stop->name;
            vertex_id_point.set_vertex_id(vertex_id);

            *data_base_.mutable_router_core()->mutable_graphs()->add_graph_vertex() = vertex_id_point;
        }
    }

    void Serialization::SaveGraphEdge() {
        auto edge = tr_.GetGraphEdges();

        for (const auto travel_prop: edge) {
            router_proto::Travel_Duration travel_duration_proto;
            travel_duration_proto.set_stop_number(travel_prop.travel_duration.stops_number);
            travel_duration_proto.set_waiting_time(travel_prop.travel_duration.waiting_time);
            travel_duration_proto.set_travel_time(travel_prop.travel_duration.travel_time);

            router_proto::Travel_Props travel_props_proto;
            *travel_props_proto.mutable_stop_name_from() = travel_prop.from->name;
            *travel_props_proto.mutable_stop_name_to() = travel_prop.to->name;
            *travel_props_proto.mutable_bus_name() = travel_prop.route->bus_name;
            *travel_props_proto.mutable_travel_duration() = travel_duration_proto;

            *data_base_.mutable_router_core()->mutable_graphs()->add_graph_edges() = travel_props_proto;
        }

    }

    void Serialization::SaveGraph() {
// Add Edges
        auto edges = tr_.GetGraph().GetEdges();
        for (const auto edge: edges) {
            router_proto::Travel_Duration weight_graph_proto;
            weight_graph_proto.set_stop_number(edge.weight.stops_number);
            weight_graph_proto.set_travel_time(edge.weight.travel_time);
            weight_graph_proto.set_waiting_time(edge.weight.waiting_time);

            router_proto::Edge edge_proto;
            edge_proto.set_edge_from(edge.from);
            edge_proto.set_edge_to(edge.to);
            *edge_proto.mutable_weight_graph() = weight_graph_proto;

            *data_base_.mutable_router_core()->mutable_graphs()->mutable_graph()->add_edges_in_graph() = edge_proto;
        }
// Add IncidenceLists
        auto incidence_lists = tr_.GetGraph().GetIncidenceLists();
        for (int i = 0; i < incidence_lists.size(); ++i) {
            router_proto::Incidence_List incidence_list_proto;
            for (int y = 0; y < incidence_lists[i].size(); ++y) {
                incidence_list_proto.add_incidence_list(incidence_lists[i][y]);
            }
            *data_base_.mutable_router_core()->mutable_graphs()->mutable_graph()->add_incidence_lists() = incidence_list_proto;
        }
    }

    void Serialization::LoadRouteSetting() {
        domain::RouteSettings settings;

        settings.bus_velocity = data_base_.router_core().route_setting().bus_velocity();
        settings.bus_wait_time = data_base_.router_core().route_setting().bus_wait_time();
        tr_.SetRouteSettings(settings);
    }

// Load methods
    void Serialization::LoadGraphs() {
        LoadGraphVertex();
        LoadGraphEdge();
        LoadGraph();
    }

    void Serialization::LoadGraphVertex() {
        std::unordered_map<domain::StopPtr, graph::VertexId> graph_vertexes;

        for (int i = data_base_.router_core().graphs().graph_vertex_size() - 1; i >= 0; --i) {
            auto vertex = data_base_.router_core().graphs().graph_vertex(i);
            graph_vertexes[tc_.FindStop(vertex.stop_name())] = vertex.vertex_id();
        }
        tr_.SetGraphVertex(graph_vertexes);

    }

    void Serialization::LoadGraphEdge() {

        std::vector<transport_router::TravelProps> graph_edges;

        for (int i = 0; i < data_base_.router_core().graphs().graph_edges_size(); ++i) {
            auto edges = data_base_.router_core().graphs().graph_edges(i);
            transport_router::TravelProps travel_props;

            travel_props.from = tc_.FindStop(edges.stop_name_from());
            travel_props.to = tc_.FindStop(edges.stop_name_to());
            travel_props.route = tc_.FindBus(edges.bus_name());

            travel_props.travel_duration.stops_number = edges.travel_duration().stop_number();
            travel_props.travel_duration.waiting_time = edges.travel_duration().waiting_time();
            travel_props.travel_duration.travel_time = edges.travel_duration().travel_time();

            graph_edges.push_back(travel_props);
        }

        tr_.SetGraphEdges(graph_edges);
    }

    void Serialization::LoadGraph() {
        std::vector<graph::Edge<transport_router::TravelDuration>> edges;

        auto graph_proto = data_base_.router_core().graphs().graph();
        for (int i = 0; i < graph_proto.edges_in_graph_size(); ++i) {
            auto edge_proto = graph_proto.edges_in_graph(i);
            graph::Edge<transport_router::TravelDuration> edge;

            edge.from = edge_proto.edge_from();
            edge.to = edge_proto.edge_to();

            edge.weight.stops_number = edge_proto.weight_graph().stop_number();
            edge.weight.travel_time = edge_proto.weight_graph().travel_time();
            edge.weight.waiting_time = edge_proto.weight_graph().waiting_time();

            edges.push_back(edge);
        }

        std::vector<std::vector<graph::EdgeId>> incidence_lists;

        for (int i = 0; i < graph_proto.incidence_lists_size(); ++i) {
            auto list_proto = graph_proto.incidence_lists(i);
            std::vector<graph::EdgeId> incidence_list;
            for (int y = 0; y < list_proto.incidence_list_size(); ++y) {
                incidence_list.push_back(list_proto.incidence_list(y));
            }
            incidence_lists.push_back(incidence_list);
        }

        graph::DirectedWeightedGraph<transport_router::TravelDuration> graph;
        graph.SetEdge(edges);
        graph.SetIncidenceList(incidence_lists);

        tr_.SetGraph(graph);
    }
}

