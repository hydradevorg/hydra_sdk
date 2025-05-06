#include <hydra_address/geohash.hpp>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <bitset>

namespace hydra {
namespace address {

// Base32 encoding alphabet for geohash
static const char* BASE32_CHARS = "0123456789bcdefghjkmnpqrstuvwxyz";

// Geohash implementation
Geohash::Geohash() : m_precision(GeohashPrecision::LEVEL_9) {}

Geohash::Geohash(GeohashPrecision precision) : m_precision(precision) {}

std::string Geohash::encode(const GeoCoordinates& coords) const {
    return encode(coords.latitude, coords.longitude);
}

std::string Geohash::encode(double latitude, double longitude) const {
    int precision = static_cast<int>(m_precision);
    return encodeImpl(latitude, longitude, precision);
}

std::optional<GeoCoordinates> Geohash::decode(const std::string& geohash) const {
    return decodeImpl(geohash);
}

GeohashPrecision Geohash::getPrecision() const {
    return m_precision;
}

void Geohash::setPrecision(GeohashPrecision precision) {
    m_precision = precision;
}

std::optional<std::array<double, 4>> Geohash::getBoundingBox(const std::string& geohash) const {
    auto coords = decode(geohash);
    if (!coords) {
        return std::nullopt;
    }
    
    // Calculate the bounding box based on the precision
    int precision = geohash.length();
    return calculateBoundingBox(coords->latitude, coords->longitude, precision);
}

std::optional<std::array<std::string, 8>> Geohash::getNeighbors(const std::string& geohash) const {
    if (!isValid(geohash)) {
        return std::nullopt;
    }
    
    auto bbox = getBoundingBox(geohash);
    if (!bbox) {
        return std::nullopt;
    }
    
    double lat = (bbox->at(0) + bbox->at(2)) / 2.0;
    double lon = (bbox->at(1) + bbox->at(3)) / 2.0;
    
    double lat_err = bbox->at(2) - lat;
    double lon_err = bbox->at(3) - lon;
    
    std::array<std::string, 8> neighbors;
    
    // North
    neighbors[0] = encode(lat + 2 * lat_err, lon);
    
    // Northeast
    neighbors[1] = encode(lat + 2 * lat_err, lon + 2 * lon_err);
    
    // East
    neighbors[2] = encode(lat, lon + 2 * lon_err);
    
    // Southeast
    neighbors[3] = encode(lat - 2 * lat_err, lon + 2 * lon_err);
    
    // South
    neighbors[4] = encode(lat - 2 * lat_err, lon);
    
    // Southwest
    neighbors[5] = encode(lat - 2 * lat_err, lon - 2 * lon_err);
    
    // West
    neighbors[6] = encode(lat, lon - 2 * lon_err);
    
    // Northwest
    neighbors[7] = encode(lat + 2 * lat_err, lon - 2 * lon_err);
    
    return neighbors;
}

std::optional<double> Geohash::getDistance(const std::string& geohash1, const std::string& geohash2) const {
    auto coords1 = decode(geohash1);
    auto coords2 = decode(geohash2);
    
    if (!coords1 || !coords2) {
        return std::nullopt;
    }
    
    return haversineDistance(coords1->latitude, coords1->longitude, coords2->latitude, coords2->longitude);
}

bool Geohash::isValid(const std::string& geohash) const {
    if (geohash.empty()) {
        return false;
    }
    
    // Check if all characters are valid base32 characters
    for (char c : geohash) {
        if (strchr(BASE32_CHARS, c) == nullptr) {
            return false;
        }
    }
    
    return true;
}

std::vector<uint8_t> Geohash::encodeWithData(const std::vector<uint8_t>& data, const GeoCoordinates& coords) const {
    std::string geohash = encode(coords);
    
    // Create the result vector
    std::vector<uint8_t> result;
    
    // Add geohash length (1 byte)
    result.push_back(static_cast<uint8_t>(geohash.length()));
    
    // Add geohash
    result.insert(result.end(), geohash.begin(), geohash.end());
    
    // Add data
    result.insert(result.end(), data.begin(), data.end());
    
    return result;
}

std::optional<std::pair<std::vector<uint8_t>, GeoCoordinates>> Geohash::decodeWithData(const std::vector<uint8_t>& data) const {
    if (data.empty()) {
        return std::nullopt;
    }
    
    // Extract geohash length
    uint8_t geohash_length = data[0];
    
    // Check if the data is long enough
    if (data.size() < 1 + geohash_length) {
        return std::nullopt;
    }
    
    // Extract geohash
    std::string geohash(data.begin() + 1, data.begin() + 1 + geohash_length);
    
    // Decode geohash to coordinates
    auto coords = decode(geohash);
    if (!coords) {
        return std::nullopt;
    }
    
    // Extract the remaining data
    std::vector<uint8_t> result_data(data.begin() + 1 + geohash_length, data.end());
    
    return std::make_pair(result_data, *coords);
}

// Helper methods
std::string Geohash::encodeImpl(double latitude, double longitude, int precision) const {
    // Validate input
    if (latitude < -90.0 || latitude > 90.0 || longitude < -180.0 || longitude > 180.0) {
        throw std::invalid_argument("Invalid coordinates");
    }
    
    // Normalize longitude to [0, 360)
    longitude = longitude < 0 ? longitude + 360.0 : longitude;
    
    // Normalize latitude to [0, 180)
    latitude += 90.0;
    
    // Bounds for binary search
    double lat_min = 0.0, lat_max = 180.0;
    double lon_min = 0.0, lon_max = 360.0;
    
    std::string result;
    int bit = 0;
    int ch = 0;
    
    while (result.length() < static_cast<size_t>(precision)) {
        if (bit % 5 == 0) {
            if (bit > 0) {
                result += BASE32_CHARS[ch];
            }
            ch = 0;
        }
        
        // Even bits are longitude, odd bits are latitude
        if (bit % 2 == 0) {
            double mid = (lon_min + lon_max) / 2.0;
            if (longitude >= mid) {
                ch |= (1 << (4 - (bit % 5)));
                lon_min = mid;
            } else {
                lon_max = mid;
            }
        } else {
            double mid = (lat_min + lat_max) / 2.0;
            if (latitude >= mid) {
                ch |= (1 << (4 - (bit % 5)));
                lat_min = mid;
            } else {
                lat_max = mid;
            }
        }
        
        bit++;
        
        // Add the last character if we've processed all bits
        if (bit % 5 == 0 && result.length() == static_cast<size_t>(precision) - 1) {
            result += BASE32_CHARS[ch];
        }
    }
    
    return result;
}

std::optional<GeoCoordinates> Geohash::decodeImpl(const std::string& geohash) const {
    if (!isValid(geohash)) {
        return std::nullopt;
    }
    
    // Bounds for binary search
    double lat_min = -90.0, lat_max = 90.0;
    double lon_min = -180.0, lon_max = 180.0;
    
    bool is_even = true;
    
    for (char c : geohash) {
        const char* p = strchr(BASE32_CHARS, c);
        if (!p) {
            return std::nullopt;
        }
        
        int char_index = p - BASE32_CHARS;
        
        // Process each bit in the character
        for (int i = 4; i >= 0; --i) {
            int bit = (char_index >> i) & 1;
            
            if (is_even) {
                // Longitude
                double mid = (lon_min + lon_max) / 2.0;
                if (bit == 1) {
                    lon_min = mid;
                } else {
                    lon_max = mid;
                }
            } else {
                // Latitude
                double mid = (lat_min + lat_max) / 2.0;
                if (bit == 1) {
                    lat_min = mid;
                } else {
                    lat_max = mid;
                }
            }
            
            is_even = !is_even;
        }
    }
    
    // Calculate the center of the bounding box
    double latitude = (lat_min + lat_max) / 2.0;
    double longitude = (lon_min + lon_max) / 2.0;
    
    return GeoCoordinates{latitude, longitude, 0.0};
}

std::array<double, 4> Geohash::calculateBoundingBox(double latitude, double longitude, int precision) const {
    // Calculate the error based on precision
    double lat_err = 90.0 / pow(2.0, precision * 2.5);
    double lon_err = 180.0 / pow(2.0, precision * 2.5);
    
    // Calculate the bounding box
    double lat_min = std::max(-90.0, latitude - lat_err);
    double lat_max = std::min(90.0, latitude + lat_err);
    double lon_min = std::max(-180.0, longitude - lon_err);
    double lon_max = std::min(180.0, longitude + lon_err);
    
    return {lat_min, lon_min, lat_max, lon_max};
}

double Geohash::haversineDistance(double lat1, double lon1, double lat2, double lon2) const {
    // Earth radius in meters
    const double R = 6371000.0;
    
    // Convert to radians
    lat1 = lat1 * M_PI / 180.0;
    lon1 = lon1 * M_PI / 180.0;
    lat2 = lat2 * M_PI / 180.0;
    lon2 = lon2 * M_PI / 180.0;
    
    // Haversine formula
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;
    double a = sin(dlat / 2.0) * sin(dlat / 2.0) + cos(lat1) * cos(lat2) * sin(dlon / 2.0) * sin(dlon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
    double distance = R * c;
    
    return distance;
}

} // namespace address
} // namespace hydra
