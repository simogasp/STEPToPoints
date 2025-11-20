#pragma once
#include <string>
#include <vector>
#include <optional>

/**
 * @brief Generates a vector of indices from a range.
 *
 * @param start The start index (1-based, inclusive).
 * @param end The end index (1-based, inclusive).
 * @return std::vector<std::size_t> Vector of zero-based indices.
 */
auto generateRange(std::size_t start, std::size_t end) -> std::vector<std::size_t>;

/**
 * @brief Validates that a string is non-empty and starts with a digit.
 *
 * @param str The string to validate.
 * @return bool True if valid, false otherwise.
 */
auto validateRangeString(const std::string& str) -> bool;

/**
 * @brief Parses a string into a size_t value with validation.
 *
 * @param str The string to parse.
 * @param fullInput The original full input string for error messages.
 * @return std::optional<std::size_t> The parsed value if valid, std::nullopt otherwise.
 */
auto parseRangeValue(const std::string& str, const std::string& fullInput) -> std::optional<std::size_t>;

/**
 * @brief Parses a range string (e.g., "3-7") into start and end indices.
 *
 * @param sel The string to parse.
 * @return std::optional<std::pair<std::size_t, std::size_t>> Pair of (start, end) if valid, std::nullopt otherwise.
 */
auto parseRange(const std::string& sel) -> std::optional<std::pair<std::size_t, std::size_t>>;

/**
 * @brief Parses a single index string (e.g., "3") into a 1-based index.
 *
 * @param sel The string to parse.
 * @return std::optional<std::size_t> The 1-based index if valid, std::nullopt otherwise.
 */
auto parseSingleIndex(const std::string& sel) -> std::optional<std::size_t>;

/**
 * @brief Attempts to parse a string as a solid index (1-based) or a range.
 *
 * @param sel The string to parse containing either an index (e.g."3") or a range (e.g."3-7").
 * @param maxIndex The maximum valid index (size of the namedSolids vector).
 * @return std::optional<std::vector<size_t>> Vector of zero-based indices if valid, std::nullopt otherwise.
 */
auto parseSolidIndex(const std::string& sel, std::size_t maxIndex) -> std::optional<std::vector<std::size_t>>;