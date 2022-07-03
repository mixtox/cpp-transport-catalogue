#pragma once

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

#include <memory>
#include "log_duration.h"


namespace transport_router {

    struct TravelDuration {
        TravelDuration() = default;

        TravelDuration(int stops_number, double waiting_time, double travel_time)
                : stops_number(stops_number), waiting_time(waiting_time), travel_time(travel_time) {
        }

        int stops_number = 0;
        double waiting_time = 0.0;
        double travel_time = 0.0;
    };

    struct TravelProperties {
        domain::StopPtr from = nullptr;
        domain::StopPtr to = nullptr;
        domain::BusPtr route = nullptr;
        TravelDuration travel_duration = {};
    };

    TravelDuration operator+(const TravelDuration& lhs, const TravelDuration& rhs);

    bool operator<(const TravelDuration& lhs, const TravelDuration& rhs);

    bool operator>(const TravelDuration& lhs, const TravelDuration& rhs);

    class DistanceCalc {
    public:

        DistanceCalc(const transport_catalogue::TransportCatalogue& catalogue, const domain::BusPtr bus);
        size_t DistanceBetweenStop(size_t index_from, size_t index_to);

    private:

        std::vector<size_t> forward_distance_;
        std::vector<size_t> reverse_distance_;

    };


    class TransportRouter {
    public:

        explicit TransportRouter(const transport_catalogue::TransportCatalogue &catalogue);

        void RouteInit(const domain::RouteSettings &settings);

        std::optional<std::vector<const transport_router::TravelProperties*>> FindRoute(std::string_view from, std::string_view to) const;

    private:

        domain::RouteSettings settings_;
        const transport_catalogue::TransportCatalogue &tc_;
        std::unique_ptr<graph::DirectedWeightedGraph<TravelDuration>> graph_;
        std::unordered_map<domain::StopPtr, graph::VertexId> graph_vertexes_;
        std::vector<TravelProperties> graph_edges_;
        std::unique_ptr<graph::Router<TravelDuration>> router_;

        void SetRouteSettings(const domain::RouteSettings &settings);

        void FillGraphStop();
        void FillGraphBuses();

    };


}

