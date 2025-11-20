#include "solid_index_parser.hpp"
#include <cctype>
#include <exception>
#include <iostream>

auto generateRange(std::size_t start, std::size_t end) -> std::vector<std::size_t>
{
    std::vector<std::size_t> indices;
    indices.reserve(end - start + 1);
    for(auto i = start; i <= end; ++i)
    {
        indices.push_back(i - 1); // Convert to zero-based
    }
    return indices;
}


auto validateRangeString(const std::string& str) -> bool
{
    return !str.empty() && std::isdigit(static_cast<unsigned char>(str[0]));
}


auto parseRangeValue(const std::string& str, const std::string& fullInput) -> std::optional<std::size_t>
{
    try
    {
        std::size_t pos = 0;
        const auto value = std::stoul(str, &pos);

        // Ensure the entire string was consumed (no trailing characters)
        if(pos != str.length())
        {
            std::cerr << "Invalid range format: " << fullInput << "\n";
            return std::nullopt;
        }

        return value;
    }
    catch(const std::invalid_argument&)
    {
        std::cerr << "Invalid range format: " << fullInput << "\n";
    }
    catch(const std::out_of_range&)
    {
        std::cerr << "Range values out of bounds: " << fullInput << "\n";
    }
    return std::nullopt;
}


auto validateRangeBounds(std::size_t start, std::size_t end) -> bool
{
    return start >= 1 && start <= end;
}


auto parseRange(const std::string& sel) -> std::optional<std::pair<std::size_t, std::size_t>>
{
    const auto dashPos = sel.find('-');
    if(dashPos == std::string::npos || dashPos == 0)
    {
        return std::nullopt;
    }

    const auto startStr = sel.substr(0, dashPos);
    const auto endStr = sel.substr(dashPos + 1);

    if(!validateRangeString(startStr) || !validateRangeString(endStr))
    {
        std::cerr << "Invalid range format: " << sel << "\n";
        return std::nullopt;
    }

    const auto start = parseRangeValue(startStr, sel);
    const auto end = parseRangeValue(endStr, sel);

    if(!start.has_value() || !end.has_value())
    {
        return std::nullopt;
    }

    if(validateRangeBounds(start.value(), end.value()))
    {
        return {{start.value(), end.value()}};
    }

    return std::nullopt;
}


auto parseSingleIndex(const std::string& sel) -> std::optional<std::size_t>
{
    // Check for empty string or leading whitespace/non-digit
    if(sel.empty() || !std::isdigit(static_cast<unsigned char>(sel[0])))
    {
        std::cerr << "Invalid index provided: " << sel << "\n";
        return std::nullopt;
    }

    try
    {
        std::size_t pos = 0;
        const auto index = std::stoul(sel, &pos);

        // Ensure the entire string was consumed (no trailing characters)
        if(pos != sel.length())
        {
            std::cerr << "Invalid index provided: " << sel << "\n";
            return std::nullopt;
        }

        if(index >= 1)
        {
            return index;
        }
        std::cerr << "Index must be at least 1: " << sel << "\n";
    }
    catch(const std::invalid_argument&)
    {
        std::cerr << "Invalid index provided: " << sel << "\n";
    }
    catch(const std::out_of_range&)
    {
        std::cerr << "Index out of range: " << sel << "\n";
    }
    return std::nullopt;
}


auto parseSolidIndex(const std::string& sel, std::size_t maxIndex) -> std::optional<std::vector<std::size_t>>
{
    // Try parsing as range first
    if(const auto range = parseRange(sel))
    {
        const auto [start, end] = range.value();

        if(end > maxIndex)
        {
            std::cerr << "Range end exceeds max index: " << end << " > " << maxIndex << "\n";
            return std::nullopt;
        }

        return generateRange(start, end);
    }

    // Try parsing as single index
    if(const auto index = parseSingleIndex(sel); index.has_value())
    {
        if(index.value() <= maxIndex)
        {
            return {{index.value() - 1}};
        }
        std::cerr << "Index out of valid range: " << sel << "\n";
    }

    return std::nullopt;
}


