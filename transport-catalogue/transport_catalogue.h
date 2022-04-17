#pragma once
#include "geo.h"

#include <deque>
#include <string>
#include <string_view>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <utility>
#include <cctype>
#include <functional>


namespace detail
{
    std::pair<std::string_view, std::string_view> Split(std::string_view, char, int count = 1);
    std::string_view DeleteLeftSpaces(std::string_view);
    std::string_view DeleteRightSpaces(std::string_view);
    std::string_view TrimString(std::string_view);
}

namespace transport_catalogue
{

    enum class InputQueryType
    {
        Undefined,
        AddStop,
        AddBus,
        AddStopsDistance,
    };

    struct InputQuery
    {
        InputQueryType type = InputQueryType::Undefined;
        std::string query;
    };

    enum class RequestQueryType
    {
        Undefined,
        GetBusByName,
        GetBusesForStop,
    };

    struct RequestQuery
    {
        RequestQueryType type;
        std::string query;
        std::string reply;
    };

    struct Stop
    {
    public:
        std::string name;
        geo::Coordinates coordinates{ 0L,0L };

        friend std::ostream& operator<<(std::ostream& out, const Stop& stop);
    };

    struct Bus
    {
        std::string bus_number;
        std::vector<const Stop*> stops;
        size_t unique_stops_qty = 0U;
        double geo_route_length = 0L;
        size_t meters_route_length = 0U;
        double curvature = 0L;

        friend std::ostream& operator<<(std::ostream& os, const Bus* route);
    };

    struct PairPointersHasher {
    public:
        size_t operator() (const std::pair<const Stop*, const Stop*> stops_pair) const {
            return hasher(stops_pair.first) + 37 * hasher(stops_pair.second);
        }
    private:
        std::hash<const void*> hasher;
    };


    class TransportCatalogue
    {
    public:
        TransportCatalogue();
        ~TransportCatalogue();

        void AddStop(Stop&&);
        void AddBus(Bus&&);

        void AddDistance(const Stop*, const Stop*, size_t);
        size_t GetDistance(const Stop*, const Stop*);
        size_t GetDistanceDirectly(const Stop*, const Stop*);

        const Stop* FindStop(std::string_view);
        Bus* FindBus(std::string_view);

        void GetBusInfo(std::string_view, std::string&);
        void GetBusesForStop(std::string_view, std::string&);

        void ProcessInputQuery(InputQuery&);

    private:
        std::deque<Stop> stops_;
        std::unordered_map<std::string_view, const Stop*> stopname_to_stop_;
        std::deque<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_bus_;
        std::unordered_map<std::pair<const Stop*, const Stop*>, size_t, PairPointersHasher> distances_map_;

        std::string_view GetStopName(const Stop* stop_ptr);
        std::string_view GetStopName(const Stop stop);

        std::string_view GetBusName(const Bus* route_ptr);
        std::string_view GetBusName(const Bus route);
    };
}
