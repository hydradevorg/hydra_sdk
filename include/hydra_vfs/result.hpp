#pragma once

#include <string>
#include <utility>
#include <optional>

namespace hydra {
namespace vfs {

/**
 * @brief Result class for operations that can fail
 *
 * This class represents the result of an operation that can either succeed with a value
 * or fail with an error message.
 *
 * @tparam T Type of the value in case of success
 */
template <typename T>
class Result {
public:
    /**
     * @brief Create a successful result
     *
     * @param value Value to return
     * @return Result<T> Successful result
     */
    static Result<T> success(T value) {
        Result<T> result;
        result.m_value = std::move(value);
        return result;
    }

    /**
     * @brief Create a failed result
     *
     * @param error Error message
     * @return Result<T> Failed result
     */
    static Result<T> error(std::string error) {
        Result<T> result;
        result.m_error = std::move(error);
        return result;
    }

    /**
     * @brief Check if the result is successful
     *
     * @return bool True if successful, false otherwise
     */
    bool success() const {
        return m_value.has_value();
    }

    /**
     * @brief Get the value
     *
     * @return const T& Value
     * @throws std::runtime_error if the result is not successful
     */
    const T& value() const {
        if (!m_value.has_value()) {
            throw std::runtime_error("Attempted to access value of failed result: " + m_error);
        }
        return m_value.value();
    }

    /**
     * @brief Get the error message
     *
     * @return const std::string& Error message
     * @throws std::runtime_error if the result is successful
     */
    const std::string& error() const {
        if (m_value.has_value()) {
            throw std::runtime_error("Attempted to access error of successful result");
        }
        return m_error;
    }

private:
    /**
     * @brief Default constructor for internal use
     */
    Result() = default;

    std::optional<T> m_value;
    std::string m_error;
};

} // namespace vfs
} // namespace hydra
