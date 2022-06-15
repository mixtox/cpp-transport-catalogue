#pragma once

namespace geo {

inline const double EARTH_RADIUS = 6371000;
    
struct Coordinates {
    double lat; // Широта
    double lng; // Долгота
    bool operator==(const Coordinates& other) const;
    bool operator!=(const Coordinates& other) const;
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo
