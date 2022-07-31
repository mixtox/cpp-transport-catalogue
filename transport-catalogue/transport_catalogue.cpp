#include "transport_catalogue.h"

namespace transport_catalogue {

    void TransportCatalogue::AddStop(std::string_view stop_name, geo::Coordinates coordinates) {
        const auto &stop = stops_.emplace_back(domain::Stop{std::string(stop_name), coordinates});
        stop_binding_.insert({stop.name, &stop});
        buses_in_stop_[&stop];
    }

    domain::StopPtr TransportCatalogue::FindStop(std::string_view stop_name) const{
        const auto it = stop_binding_.find(stop_name);
        if (it == stop_binding_.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    void TransportCatalogue::AddBus(std::string_view bus_name, std::vector<std::string> &stops, bool is_circle) {
        std::vector<domain::StopPtr> route_stops;
        route_stops.reserve(stops.size());

        auto &route = bus_.emplace_back(domain::Bus{std::string(bus_name), route_stops, is_circle});
        bus_binding_.insert({route.bus_name, &route});
        for (const auto &stop: stops) {
            const auto *stop_find = FindStop(stop);
            route.bus_stops.push_back(stop_find);
            buses_in_stop_[stop_find].insert(FindBus(bus_name));
        }
    }

    domain::BusPtr TransportCatalogue::FindBus(std::string_view bus_name) const {
        const auto it = bus_binding_.find(bus_name);
        if (it == bus_binding_.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    const std::optional<Buses_Info> TransportCatalogue::GetBusInfo(std::string input) const {
        Buses_Info result;
        const auto *buses_info = FindBus(input);
        if (buses_info == nullptr) {
            return {};
        }
        std::vector<domain::StopPtr> stops_in_route{buses_info->bus_stops.begin(), buses_info->bus_stops.end()};
        std::unordered_set<domain::StopPtr> unique_stop;
        std::for_each(
                stops_in_route.begin(), stops_in_route.end(),
                [&unique_stop](const auto &stop) {
                    if (!unique_stop.count(stop)) {
                        unique_stop.insert(stop);
                    }
                });
        if (!buses_info->is_circle) {
            stops_in_route.insert(stops_in_route.end(), std::next(buses_info->bus_stops.rbegin()),
                                  buses_info->bus_stops.rend());
        }
        double route_length = 0.0;
        size_t route_length_true = 0;
        for (auto it = stops_in_route.begin(); it != std::prev(stops_in_route.end()); ++it) {
            route_length += ComputeDistance((*it)->coordinates, (*(it + 1))->coordinates);
            route_length_true += GetDistance(*it, *(it + 1));
        }
        result.route_name = buses_info->bus_name;
        result.stop_count = static_cast<int>(stops_in_route.size());
        result.unique_stop = static_cast<int>(unique_stop.size());
        result.route_length = route_length_true;
        result.curvature = route_length_true / route_length;

        return result;
    }

    const std::unordered_set<domain::BusPtr>* TransportCatalogue::GetStopInfo(std::string_view input) const{
        if (FindStop(input)) {
            return &buses_in_stop_.at(FindStop(input));
        } else {
            return nullptr;
        }
    }

    void TransportCatalogue::SetDistanceStops(domain::StopPtr first_stop, domain::StopPtr second_stop, double distance) {
        std::pair<domain::StopPtr, domain::StopPtr> stop_pair = std::make_pair(first_stop, second_stop);
        distance_between_stop_[stop_pair] = distance;
    }

    size_t TransportCatalogue::GetDistance(domain::StopPtr first_stop, domain::StopPtr second_stop) const{
        auto pair_stop = std::make_pair(first_stop, second_stop);
        if (distance_between_stop_.find(pair_stop) != distance_between_stop_.end()) {
            return distance_between_stop_.at(pair_stop);
        }
        return GetDistance(second_stop, first_stop);
    }

    const std::map<std::string_view, domain::BusPtr> TransportCatalogue::GetAllBusesInfo() const {
        std::map<std::string_view, domain::BusPtr> result;
        for (const auto& bus : bus_) {
            result[bus.bus_name] = &bus;
        }
        return result;
    }

    const std::unordered_map<std::string_view, domain::StopPtr> TransportCatalogue::GetAllStop() const {
        return stop_binding_;
    }

    const std::deque<domain::Stop> TransportCatalogue::GetStopBase() const {
        return stops_;
    }

    const std::deque<domain::Bus> TransportCatalogue::GetBusBase() const {
        return bus_;
    }

    const std::unordered_map<domain::StopPtr, std::unordered_set<domain::BusPtr>>
    TransportCatalogue::GetBusesInStopBase() const {
        return buses_in_stop_;
    }

    const std::unordered_map<std::pair<domain::StopPtr, domain::StopPtr>, double, detail::DistanceHasher>
    TransportCatalogue::GetDistanceBase() const {
        return distance_between_stop_;
    }

    size_t detail::DistanceHasher::operator()(const std::pair<domain::StopPtr, domain::StopPtr> stops_pair) const {
        return hasher(stops_pair.first) + hasher(stops_pair.second) * 37;
    }
}
