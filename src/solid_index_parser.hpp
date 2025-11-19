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
 * @brief Parses a range string (e.g., "3-7") into start and end indices.
 *
 * @param sel The string to parse.
 * @return std::optional<std::pair<std::size_t, std::size_t>> Pair of (start, end) if valid, std::nullopt otherwise.
 */
auto parseRange(const std::string& sel) -> std::optional<std::pair<std::size_t, std::size_t>>;

/**
 * @brief Attempts to parse a string as a solid index (1-based) or a range.
 *
 * @param sel The string to parse containing either an index (e.g."3") or a range (e.g."3-7").
 * @param maxIndex The maximum valid index (size of the namedSolids vector).
 * @return std::optional<std::vector<size_t>> Vector of zero-based indices if valid, std::nullopt otherwise.
 */
auto parseSolidIndex(const std::string& sel, std::size_t maxIndex) -> std::optional<std::vector<std::size_t>>;