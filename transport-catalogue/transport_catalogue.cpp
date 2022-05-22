#include "transport_catalogue.h"

namespace transport_catalogue
{


    TransportCatalogue::TransportCatalogue()
    {}

    TransportCatalogue::~TransportCatalogue()
    {}

    void TransportCatalogue::AddStop(Stop&& stop)
    {
        if (stopname_to_stop_.count(GetStopName(&stop)) == 0)
        {
            auto& ref = stops_.emplace_back(std::move(stop));
            stopname_to_stop_.insert({ std::string_view(ref.name), &ref });
        }
    }

    void TransportCatalogue::AddBus(Bus&& bus)
    {
        if (busname_to_bus_.count(GetBusName(&bus)) == 0)
        {
            auto& ref = buses_.emplace_back(std::move(bus));
            busname_to_bus_.insert({ std::string_view(ref.bus_number), &ref });
            
            std::vector<const Stop*> tmp = ref.stops;
            std::sort(tmp.begin(), tmp.end());
            auto last = std::unique(tmp.begin(), tmp.end());

            ref.unique_stops_qty = (last != tmp.end() ? std::distance(tmp.begin(), last) : tmp.size());

            int stops_num = static_cast<int>(ref.stops.size());
            if (stops_num > 1)
            {
                ref.geo_route_length = 0L;
                ref.meters_route_length = 0U;
                for (int i = 0; i < stops_num - 1; ++i)
                {
                    ref.geo_route_length += ComputeDistance(ref.stops[i]->coordinates, ref.stops[i + 1]->coordinates);
                    ref.meters_route_length += GetDistance(ref.stops[i], ref.stops[i + 1]);
                    buses_in_stop_[ref.stops[i]].push_back(&ref);
                }
                ref.curvature = ref.meters_route_length / ref.geo_route_length;
            }
            else
            {
                ref.geo_route_length = 0L;
                ref.meters_route_length = 0U;
                ref.curvature = 1L;
            }
        }
    }

    void TransportCatalogue::AddDistance(const Stop* stop_from, const Stop* stop_to, size_t dist)
    {
        if (stop_from != nullptr && stop_to != nullptr)
        {
            distances_map_.insert({ { stop_from, stop_to }, dist });
        }
    }

    size_t TransportCatalogue::GetDistance(const Stop* stop_from, const Stop* stop_to)
    {
        size_t result = GetDistanceDirectly(stop_from, stop_to);
        return (result > 0 ? result : GetDistanceDirectly(stop_to, stop_from));
    }

    size_t TransportCatalogue::GetDistanceDirectly(const Stop* stop_from, const Stop* stop_to)
    {
        if (distances_map_.count({ stop_from, stop_to }) > 0)
        {
            return distances_map_.at({ stop_from, stop_to });
        }
        else
        {
            return 0U;
        }
    }

    std::string_view TransportCatalogue::GetStopName(const Stop* stop_ptr)
    {
        return std::string_view(stop_ptr->name);
    }

    std::string_view TransportCatalogue::GetStopName(const Stop stop)
    {
        return std::string_view(stop.name);
    }

    std::string_view TransportCatalogue::GetBusName(const Bus* bus_ptr)
    {
        return std::string_view(bus_ptr->bus_number);
    }

    std::string_view TransportCatalogue::GetBusName(const Bus bus)
    {
        return std::string_view(bus.bus_number);
    }


    const Stop* TransportCatalogue::FindStop(std::string_view stop_name)
    {
        if (stopname_to_stop_.count(stop_name) == 0)
        {
            return nullptr;
        }
        else
        {
            return stopname_to_stop_.at(stop_name);
        }
    }

    Bus* TransportCatalogue::FindBus(std::string_view bus_name)
    {
        if (busname_to_bus_.count(bus_name) == 0)
        {
            return nullptr;
        }
        else
        {
            return busname_to_bus_.at(bus_name);
        }
    }


    std::vector<Bus*> TransportCatalogue::GetBusesForStop(std::string_view stop_name)
    {
        if (buses_in_stop_.count(FindStop(stop_name)))
        {
            return buses_in_stop_.at(FindStop(stop_name));
        }
        else
        {
            return {};
        }
    }
}
