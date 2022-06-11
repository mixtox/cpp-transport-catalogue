#include "domain.h"

namespace domain 
{

    size_t DistanceHasher::operator()(const std::pair<domain::StopPtr, domain::StopPtr> stops_pair) const {
        return hasher(stops_pair.first) + hasher(stops_pair.second) * 37;
    }

}