#include "stat_reader.h"

namespace transport_catalogue::stat_reader
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

    std::ostream& ProcessRequests(TransportCatalogue& trans_cat_, std::istream& is, std::ostream& os)
    {
        std::string line;
        std::getline(is, line);

        size_t request_num = static_cast<size_t>(std::stoul(line));

        for (size_t i = 0; i < request_num; ++i)
        {
            std::getline(is, line, '\n');

            auto tmp = detail::Split(line, ' ');

            tmp.first = detail::TrimString(tmp.first);
            tmp.second = detail::TrimString(tmp.second);

            if (tmp.second.empty())
            {
                continue;
            }

            using namespace std::literals;
            RequestQuery request_query;

            if (tmp.first == "Bus"sv)
            {
                request_query.type = RequestQueryType::GetBusByName;
            }
            else if (tmp.first == "Stop"sv)
            {
                request_query.type = RequestQueryType::GetBusesForStop;
            }
            else
            {
                request_query.type = RequestQueryType::Undefined;
            }

            request_query.query = tmp.second;

            ExecuteRequest(trans_cat_, request_query, std::cout);
        }

        return os;
    }

    std::ostream& ExecuteRequest(TransportCatalogue& trans_cat_, RequestQuery& request_query, std::ostream& os)
    {
        using namespace std::literals;

        switch (request_query.type)
        {
        case RequestQueryType::GetBusByName:
        {
            if (request_query.query.size() != 0)
            {
                
                request_query.query = std::string(request_query.query);
                Bus* bus_ptr = trans_cat_.FindBus(request_query.query);
                if (bus_ptr != nullptr)
                {
                    std::stringstream ss;
                    ss << bus_ptr;
                    os << ss.str() << std::endl;

                }
                else
                {
                    using namespace std::string_literals;

                    os << "Bus "s + std::string(request_query.query) + ": not found"s << std::endl;
                }
            }
        }
            break;

        case RequestQueryType::GetBusesForStop:
        {
            if (request_query.query.size() != 0)
            {
                request_query.query = std::string(request_query.query);
                if (trans_cat_.FindStop(request_query.query))
                {
                    auto bus_vec = trans_cat_.GetBusesForStop(request_query.query);

                    if (bus_vec.size()) {

                        sort(bus_vec.begin(), bus_vec.end(), [](const auto& left, const auto& right) {
                            return left->bus_number < right->bus_number;
                            });

                        os << "Stop " << request_query.query << ": buses";
                        for (auto bus : bus_vec)
                        {
                            std::stringstream ss;
                            ss << " " << bus->bus_number;
                            os << ss.str();
                        }
                        os << std::endl;
                    }
                    else
                    {
                        os << "Stop " << request_query.query << ": no buses" << std::endl;
                    }
                }
                else
                {
                    os << "Stop " << request_query.query << ": not found" << std::endl;
                }
            }
        }
        break;

        case RequestQueryType::Undefined:
        {
            break;
        }

        
        }
        return os;
    }
}
