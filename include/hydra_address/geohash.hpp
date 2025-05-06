#pragma once

#include <string>
#include <vector>
#include <array>
#include <optional>
#include <cstdint>

namespace hydra {
namespace address {

/**
 * @brief Coordinate structure for geohashing
 */
struct GeoCoordinates {
    double latitude;
    double longitude;
    double altitude;
    
    bool operator==(const GeoCoordinates& other) const {
        return latitude == other.latitude && 
               longitude == other.longitude && 
               altitude == other.altitude;
    }
};

/**
 * @brief Geohash precision levels
 */
enum class GeohashPrecision {
    LEVEL_1 = 1,  ///< ~5000km x 5000km
    LEVEL_2 = 2,  ///< ~1250km x 625km
    LEVEL_3 = 3,  ///< ~156km x 156km
    LEVEL_4 = 4,  ///< ~39km x 19.5km
    LEVEL_5 = 5,  ///< ~4.9km x 4.9km
    LEVEL_6 = 6,  ///< ~1.2km x 0.61km
    LEVEL_7 = 7,  ///< ~153m x 153m
    LEVEL_8 = 8,  ///< ~38m x 19m
    LEVEL_9 = 9,  ///< ~4.8m x 4.8m
    LEVEL_10 = 10, ///< ~1.2m x 0.6m
    LEVEL_11 = 11, ///< ~15cm x 15cm
    LEVEL_12 = 12  ///< ~3.7cm x 1.9cm
};

/**
 * @brief Geohash class for encoding and decoding geohashes
 */
class Geohash {
public:
    /**
     * @brief Default constructor
     */
    Geohash();
    
    /**
     * @brief Constructor with precision
     * @param precision Geohash precision level
     */
    explicit Geohash(GeohashPrecision precision);
    
    /**
     * @brief Encode coordinates to a geohash string
     * @param coords Coordinates to encode
     * @return Geohash string
     */
    std::string encode(const GeoCoordinates& coords) const;
    
    /**
     * @brief Encode latitude and longitude to a geohash string
     * @param latitude Latitude
     * @param longitude Longitude
     * @return Geohash string
     */
    std::string encode(double latitude, double longitude) const;
    
    /**
     * @brief Decode a geohash string to coordinates
     * @param geohash Geohash string
     * @return Coordinates or empty if invalid
     */
    std::optional<GeoCoordinates> decode(const std::string& geohash) const;
    
    /**
     * @brief Get the precision level
     * @return Precision level
     */
    GeohashPrecision getPrecision() const;
    
    /**
     * @brief Set the precision level
     * @param precision Precision level
     */
    void setPrecision(GeohashPrecision precision);
    
    /**
     * @brief Get the bounding box for a geohash
     * @param geohash Geohash string
     * @return Array of [min_lat, min_lon, max_lat, max_lon] or empty if invalid
     */
    std::optional<std::array<double, 4>> getBoundingBox(const std::string& geohash) const;
    
    /**
     * @brief Get the neighbors of a geohash
     * @param geohash Geohash string
     * @return Array of [N, NE, E, SE, S, SW, W, NW] neighbors or empty if invalid
     */
    std::optional<std::array<std::string, 8>> getNeighbors(const std::string& geohash) const;
    
    /**
     * @brief Calculate the distance between two geohashes in meters
     * @param geohash1 First geohash
     * @param geohash2 Second geohash
     * @return Distance in meters or empty if invalid
     */
    std::optional<double> getDistance(const std::string& geohash1, const std::string& geohash2) const;
    
    /**
     * @brief Check if a geohash is valid
     * @param geohash Geohash string
     * @return True if valid
     */
    bool isValid(const std::string& geohash) const;
    
    /**
     * @brief Encode binary data with geohash
     * @param data Binary data
     * @param coords Coordinates
     * @return Geohashed binary data
     */
    std::vector<uint8_t> encodeWithData(const std::vector<uint8_t>& data, const GeoCoordinates& coords) const;
    
    /**
     * @brief Decode binary data with geohash
     * @param data Geohashed binary data
     * @return Pair of binary data and coordinates or empty if invalid
     */
    std::optional<std::pair<std::vector<uint8_t>, GeoCoordinates>> decodeWithData(const std::vector<uint8_t>& data) const;

private:
    GeohashPrecision m_precision;
    
    // Helper methods
    std::string encodeImpl(double latitude, double longitude, int precision) const;
    std::optional<GeoCoordinates> decodeImpl(const std::string& geohash) const;
    std::array<double, 4> calculateBoundingBox(double latitude, double longitude, int precision) const;
    double haversineDistance(double lat1, double lon1, double lat2, double lon2) const;
};

} // namespace address
} // namespace hydra
