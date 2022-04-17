#include "input_reader.h"

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

		std::vector<InputQuery> delayed;
		for (auto element : queries)
		{
			if (element.type == InputQueryType::AddStop)
			{
				trans_cat_.ProcessInputQuery(element);
			}
			else
			{
				delayed.push_back(std::move(element));
			}
		}

		for (auto element : delayed)
		{
			if (element.type == InputQueryType::AddStopsDistance)
			{
				trans_cat_.ProcessInputQuery(element);
			}
		}

		for (auto element : delayed)
		{
			trans_cat_.ProcessInputQuery(element);
		}
	}

}
