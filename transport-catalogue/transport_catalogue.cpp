#include "transport_catalogue.h"

namespace detail
{
    std::pair<std::string_view, std::string_view> Split(std::string_view line, char separator, int count)
    {
        size_t pos = line.find(separator);
        for (int i = 1; (i < count) && (pos != line.npos); ++i)
        {
            pos = line.find(separator, pos + 1);
        }

        std::string_view left = line.substr(0, pos);

        if (pos < line.size() && pos + 1 < line.size())
        {
            return { left, line.substr(pos + 1) };
        }
        else
        {
            return { left, std::string_view() };
        }
    }

    std::string_view DeleteLeftSpaces(std::string_view line)
    {
        while (!line.empty() && std::isspace(line[0]))
        {
            line.remove_prefix(1);
        }
        return line;
    }

    std::string_view DeleteRightSpaces(std::string_view line)
    {
        size_t line_size = line.size();

        while (!line.empty() && std::isspace(line[line_size - 1]))
        {
            line.remove_suffix(1);
            --line_size;
        }
        return line;
    }

    std::string_view TrimString(std::string_view line)
    {
        return DeleteRightSpaces(DeleteLeftSpaces(line));
    }
}


namespace transport_catalogue
{
    std::ostream& operator<<(std::ostream& os, const Stop& stop)
    {
        using namespace std::string_literals;
        os << "Stop "s << stop.name << ": "s;
        os << std::fixed << std::setprecision(6);
        os << stop.coordinates.lat << "s, ";
        os << stop.coordinates.lng << std::endl;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const Bus* bus)
    {
        using namespace std::string_literals;

        if (bus != nullptr)
        {
            os << "Bus "s << bus->bus_number << ": "s;
            if (bus->stops.size())
            {
                os << bus->stops.size() << " stops on route, "s;
                os << bus->unique_stops_qty << " unique stops, "s;
                os << bus->meters_route_length << " route length, "s;
                os << std::setprecision(6);
                os << bus->curvature << " curvature"s;
            }
            else
            {
                os << "no stops"s;
            }
        }
        return os;
    }

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

    void TransportCatalogue::AddBus(Bus&& route)
    {
        if (busname_to_bus_.count(GetBusName(&route)) == 0)
        {
            auto& ref = buses_.emplace_back(std::move(route));
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

    void TransportCatalogue::ProcessInputQuery(InputQuery& input_query)
    {
        switch (input_query.type)
        {
        case InputQueryType::AddStop:
        {
            Stop new_stop;

            auto tmp = detail::Split(input_query.query, ':');

            tmp.first = detail::TrimString(tmp.first);
            tmp.second = detail::TrimString(tmp.second);

            new_stop.name = std::string(tmp.first);

            tmp = detail::Split(tmp.second, ' ');

            tmp.first = detail::DeleteRightSpaces(tmp.first);
            tmp.second = detail::DeleteLeftSpaces(tmp.second);

            new_stop.coordinates.lat = std::stod(std::string(tmp.first));
            new_stop.coordinates.lng = std::stod(std::string(tmp.second));

            AddStop(std::move(new_stop));
            break;

        }
        
        case InputQueryType::AddBus:
        {
            Bus new_route;

            auto tmp = detail::Split(input_query.query, ':');

            tmp.first = detail::TrimString(tmp.first);
            tmp.second = detail::TrimString(tmp.second);

            new_route.bus_number = std::string(tmp.first);

            char separator = (tmp.second.find('>') == tmp.second.npos ? '-' : '>');

            std::vector<std::string_view> stops_list;

            while (tmp.second.size() != 0)
            {
                tmp = detail::Split(tmp.second, separator);

                tmp.first = detail::DeleteRightSpaces(tmp.first);
                tmp.second = detail::DeleteLeftSpaces(tmp.second);

                stops_list.push_back(tmp.first);
            }

            if ((separator == '-') && (stops_list.size() > 1))
            {
                for (int i = stops_list.size() - 2; i >= 0; --i)
                {
                    stops_list.push_back(stops_list[i]);
                }
            }

            for (auto element : stops_list)
            {
                if (stopname_to_stop_.count(element) > 0)
                {
                    new_route.stops.push_back(stopname_to_stop_.at(element));
                }
            }

            AddBus(std::move(new_route));

        }
        break;

        case InputQueryType::AddStopsDistance:
        {
            auto tmp = detail::Split(input_query.query, ':');

            tmp.first = detail::TrimString(tmp.first);
            tmp.second = detail::TrimString(tmp.second);

            const Stop* stop_from = FindStop(tmp.first);
            if (stop_from == nullptr)
            {
                return;
            }

            size_t dist = 0U;
            const Stop* stop_to = nullptr;

            while (tmp.second.size() != 0)
            {
                tmp = detail::Split(tmp.second, 'm');
                tmp.first = detail::TrimString(tmp.first);
                tmp.second = detail::DeleteLeftSpaces(tmp.second);
                dist = std::stoul(std::string(tmp.first));

                tmp = detail::Split(tmp.second, ' ');
                tmp = detail::Split(tmp.second, ',');
                tmp.first = detail::TrimString(tmp.first);
                tmp.second = detail::DeleteLeftSpaces(tmp.second);
                stop_to = FindStop(tmp.first);
                if (stop_to == nullptr)
                {
                    return;
                }

                AddDistance(stop_from, stop_to, dist);
            }
        }
        break;

        case InputQueryType::Undefined:
            break;

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

    std::string_view TransportCatalogue::GetBusName(const Bus* route_ptr)
    {
        return std::string_view(route_ptr->bus_number);
    }

    std::string_view TransportCatalogue::GetBusName(const Bus route)
    {
        return std::string_view(route.bus_number);
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


    void TransportCatalogue::GetBusInfo(std::string_view bus_name, std::string& result)
    {
        std::stringstream ss;
        Bus* r_ptr = FindBus(bus_name);
        if (r_ptr != nullptr)
        {
            ss << r_ptr;
            result = ss.str();
        }
        else
        {
            using namespace std::string_literals;

            result = "Bus "s + std::string(bus_name) + ": not found"s;
        }
    }

    void TransportCatalogue::GetBusesForStop(std::string_view stop_name, std::string& result)
    {
        const Stop* s_ptr = FindStop(stop_name);
        if (s_ptr != nullptr)
        {
            std::vector<std::string_view> found_buses_sv;
            for (auto bus : busname_to_bus_)
            {
                auto tmp = std::find_if(bus.second->stops.begin(), bus.second->stops.end(),
                    [stop_name](const Stop* curr_stop)
                    {
                        return (curr_stop->name == stop_name);
                    });
                if (tmp != bus.second->stops.end())
                {
                    found_buses_sv.push_back(bus.second->bus_number);
                }
            }

            if (found_buses_sv.size() > 0)
            {
                std::sort(found_buses_sv.begin(), found_buses_sv.end());
                std::stringstream ss;
                using namespace std::string_literals;
                for (auto element : found_buses_sv)
                {
                    ss << " "s << std::string(element);
                }
                result = "Stop "s + std::string(stop_name) + ": buses"s + ss.str();
            }
            else
            {
                using namespace std::string_literals;
                result = "Stop "s + std::string(stop_name) + ": no buses"s;
            }

        }
        else
        {
            using namespace std::string_literals;
            result = "Stop "s + std::string(stop_name) + ": not found"s;
        }
    }
}
