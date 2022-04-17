#include "stat_reader.h"

namespace transport_catalogue::stat_reader
{
    void ProcessRequest(TransportCatalogue& tc, std::istream& is)
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

            switch (request_query.type)
            {
            case RequestQueryType::GetBusByName:
                if (tmp.second.size() != 0)
                {
                    request_query.query = std::string(tmp.second);
                    tc.GetBusInfo(request_query.query, request_query.reply);
                    std::cout << request_query.reply << std::endl;
                }
                break;

            case RequestQueryType::GetBusesForStop:
                if (tmp.second.size() != 0)
                {
                    request_query.query = std::string(tmp.second);
                    tc.GetBusesForStop(request_query.query, request_query.reply);
                    std::cout << request_query.reply << std::endl;
                }
                break;

            case RequestQueryType::Undefined:
                break;
            }
        }
    }
}
