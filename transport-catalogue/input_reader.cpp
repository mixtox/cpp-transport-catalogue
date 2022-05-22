#include "input_reader.h"

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

namespace transport_catalogue::input_reader
{

    void ProcessInput(TransportCatalogue& trans_cat_, std::istream& is)
    {
        std::vector<InputQuery> queries;
        std::string line;
        std::getline(is, line);

        size_t request_num = static_cast<size_t>(std::stoul(line));
        
        for (size_t i = 0; i < request_num; ++i)
        {
            using namespace std::literals;

            std::getline(is, line, '\n');

            auto tmp = detail::Split(line, ' ');
            tmp.first = detail::TrimString(tmp.first);
            tmp.second = detail::TrimString(tmp.second);

            if (tmp.second.empty())
            {
                continue;
            }

            InputQuery query;
            if (tmp.first == "Stop"sv)
            {
                query.type = InputQueryType::AddStop;

                tmp = detail::Split(tmp.second, ',', 2);

                tmp.first = detail::DeleteRightSpaces(tmp.first);
                tmp.second = detail::DeleteLeftSpaces(tmp.second);
                if (tmp.second.size() != 0)
                {
                    auto tmp_stop_name = detail::Split(tmp.first, ':');
                    tmp_stop_name.first = detail::TrimString(tmp_stop_name.first);

                    query.query = std::string(tmp.first);
                    queries.push_back(std::move(query));

                    query = {};
                    query.type = InputQueryType::AddStopsDistance;
                    query.query = std::string(tmp_stop_name.first) + ":"s + std::string(tmp.second);
                    queries.push_back(std::move(query));
                }
                else
                {
                    query.query = std::string(tmp.first);
                    queries.push_back(std::move(query));
                }
            }
            else if (tmp.first == "Bus"sv)
            {
                query.type = InputQueryType::AddBus;
                query.query = std::string(tmp.second);
                queries.push_back(std::move(query));
            }
            else
            {
                query.type = InputQueryType::Undefined;
                query.query = std::string(tmp.second);
                queries.push_back(std::move(query));
            }
        }

        ProcessInputQueries(trans_cat_, queries);
    }

    void ProcessInputQueries(TransportCatalogue& trans_cat_, std::vector<InputQuery>& queries)
    {
        for (auto& element : queries)
        {
            if (element.type == InputQueryType::AddStop)
            {
                trans_cat_.AddStop(ProcessQueryAddStop(element.query));
            }
        }

        for (auto& element : queries)
        {
            if (element.type == InputQueryType::AddStopsDistance)
            {
                ProcessQueryAddStopsDistance(trans_cat_, element.query);
            }
        }

        for (auto& element : queries)
        {
            if (element.type == InputQueryType::AddBus)
            {
                trans_cat_.AddBus(std::move(ProcessQueryAddBus(trans_cat_, element.query)));
            }
        }
    }

    Stop ProcessQueryAddStop(std::string& query)
    {
        Stop new_stop;

        auto tmp = detail::Split(query, ':');

        tmp.first = detail::TrimString(tmp.first);
        tmp.second = detail::TrimString(tmp.second);

        new_stop.name = std::string(tmp.first);

        tmp = detail::Split(tmp.second, ' ');

        tmp.first = detail::DeleteRightSpaces(tmp.first);
        tmp.second = detail::DeleteLeftSpaces(tmp.second);

        new_stop.coordinates.lat = std::stod(std::string(tmp.first));
        new_stop.coordinates.lng = std::stod(std::string(tmp.second));

        return new_stop;
    }

    void ProcessQueryAddStopsDistance(TransportCatalogue& trans_cat_, std::string& query)
    {
        auto tmp = detail::Split(query, ':');
    
        tmp.first = detail::TrimString(tmp.first);
        tmp.second = detail::TrimString(tmp.second);

        const Stop* stop_from = trans_cat_.FindStop(tmp.first);
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
            stop_to = trans_cat_.FindStop(tmp.first);
            if (stop_to == nullptr)
            {
                return;
            }

            trans_cat_.AddDistance(stop_from, stop_to, dist);
        }
    }

    Bus ProcessQueryAddBus(TransportCatalogue& trans_cat_, std::string& query)
    {
        Bus new_bus;

        auto tmp = detail::Split(query, ':');

        tmp.first = detail::TrimString(tmp.first);
        tmp.second = detail::TrimString(tmp.second);

        new_bus.bus_number = std::string(tmp.first);

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
            for (int i = static_cast<int>(stops_list.size()) - 2; i >= 0; --i)
            {
                stops_list.push_back(stops_list[i]);
            }
        }

        for (auto& element : stops_list)
        {
            const Stop* tmp_ptr = trans_cat_.FindStop(element);
            if (tmp_ptr != nullptr)
            {
                new_bus.stops.push_back(tmp_ptr);
            }
        }
        return new_bus;
    }

}
