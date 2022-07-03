#include "transport_router.h"

namespace transport_router {

    TransportRouter::TransportRouter(const transport_catalogue::TransportCatalogue &catalogue)
            : tc_(catalogue) {}

    void TransportRouter::SetRouteSettings(const domain::RouteSettings &settings) {
        settings_.bus_velocity = settings.bus_velocity / 3.6;        //Переводим скорость из км/ч в м/с
        settings_.bus_wait_time = settings.bus_wait_time * 60;        //Переводим минуты в секунды
    }

    DistanceCalc::DistanceCalc(const transport_catalogue::TransportCatalogue &tc,
                                         const domain::BusPtr bus)
            : forward_distance_(bus->bus_stops.size()), reverse_distance_(bus->bus_stops.size()) {
        size_t forward_accum = 0;
        size_t reverse_accum = 0;
        forward_distance_[0] = 0;
        reverse_distance_[0] = 0;
        for (size_t i = 0; i < bus->bus_stops.size() - 1; ++i) {
            forward_accum += tc.GetDistance(bus->bus_stops[i], bus->bus_stops[i + 1]);
            forward_distance_[i + 1] = forward_accum;
            if (!bus->is_circle) {
                reverse_accum += tc.GetDistance(bus->bus_stops[i + 1], bus->bus_stops[i]);
                reverse_distance_[i + 1] = reverse_accum;
            }
        }
    }

    size_t DistanceCalc::DistanceBetweenStop(size_t index_from, size_t index_to) {
        if (index_from < index_to) {
            return forward_distance_[index_to] - forward_distance_[index_from];
        } else {
            return reverse_distance_[index_from] - reverse_distance_[index_to];
        }
    }

    TravelDuration operator+(const TravelDuration &lhs, const transport_router::TravelDuration &rhs) {
        return {lhs.stops_number + rhs.stops_number, lhs.waiting_time + rhs.waiting_time,
                lhs.travel_time + rhs.travel_time};
    }

    bool operator<(const TravelDuration &lhs, const transport_router::TravelDuration &rhs) {
        return (lhs.waiting_time + lhs.travel_time < rhs.waiting_time + rhs.travel_time);
    }

    bool operator>(const TravelDuration &lhs, const transport_router::TravelDuration &rhs) {
        return (lhs.waiting_time + lhs.travel_time > rhs.waiting_time + rhs.travel_time);
    }

    void TransportRouter::FillGraphStop() {
        auto all_stops = tc_.GetAllStop();
        graph_ = std::make_unique<graph::DirectedWeightedGraph<TravelDuration>>(all_stops.size());
        graph::VertexId vertex_counter = 0;

        for (const auto &[_, stop_ptr]: all_stops) {
            graph_vertexes_.insert({stop_ptr, vertex_counter++});
        }
    }

    void TransportRouter::FillGraphBuses() {
        for (const auto &[_, route_ptr]: tc_.GetAllRouteInfo()) {
            const auto &stops = route_ptr->bus_stops;

            DistanceCalc distance_calc(tc_, route_ptr);
            for (int i = 0; i < stops.size() - 1; ++i) {
                for (int j = i + 1; j < stops.size(); ++j) {
                    TravelDuration travel_dur(j - i,
                                              settings_.bus_wait_time,
                                              distance_calc.DistanceBetweenStop(i, j) / settings_.bus_velocity);
                    TravelProperties travel_unit{stops[i], stops[j], route_ptr, travel_dur};
                    graph_->AddEdge(
                            graph::Edge<TravelDuration>{graph_vertexes_[travel_unit.from],
                                                        graph_vertexes_[travel_unit.to],
                                                        travel_dur});
                    graph_edges_.push_back(std::move(travel_unit));

                    if (!route_ptr->is_circle) {
                        TravelDuration travel_dur(j - i,
                                                  settings_.bus_wait_time,
                                                  distance_calc.DistanceBetweenStop(j, i) / settings_.bus_velocity);
                        TravelProperties travel_unit{stops[i], stops[j], route_ptr, travel_dur};
                        graph_->AddEdge(graph::Edge<TravelDuration>{graph_vertexes_[travel_unit.to],
                                                                    graph_vertexes_[travel_unit.from], travel_dur});
                        graph_edges_.push_back(std::move(travel_unit));
                    }
                }
            }
        }

        router_ = std::make_unique<graph::Router<TravelDuration>>(*graph_);
    }

    void TransportRouter::RouteInit(const domain::RouteSettings &settings) {
        SetRouteSettings(settings);
        FillGraphStop();
        FillGraphBuses();
    }

    std::optional<std::vector<const TravelProperties *>>
    TransportRouter::FindRoute(std::string_view from, std::string_view to) const {

        domain::StopPtr stop_from = tc_.FindStop(from);
        domain::StopPtr stop_to = tc_.FindStop(to);

        if (stop_from == nullptr || stop_to == nullptr) {
            return std::nullopt;
        }

        std::vector<const transport_router::TravelProperties *> result;
        if (stop_from == stop_to) {
            return result;
        }

        graph::VertexId vertex_from = graph_vertexes_.at(stop_from);
        graph::VertexId vertex_to = graph_vertexes_.at(stop_to);

        auto route = router_->BuildRoute(vertex_from, vertex_to);
        if (!route.has_value()) {
            return std::nullopt;
        }

        for (const auto &edge: route.value().edges) {
            result.push_back(&graph_edges_.at(edge));
        }

        return result;
    }

}