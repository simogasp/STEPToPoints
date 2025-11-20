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


auto parseRange(const std::string& sel) -> std::optional<std::pair<std::size_t, std::size_t>>
{
    if(const auto dashPos = sel.find('-'); dashPos != std::string::npos && dashPos > 0)
    {
        try
        {
            const auto startStr = sel.substr(0, dashPos);
            const auto endStr = sel.substr(dashPos + 1);

            // Check for empty strings or strings with only whitespace
            if(startStr.empty() || endStr.empty())
            {
                std::cerr << "Invalid range format: " << sel << "\n";
                return std::nullopt;
            }

            // Check for leading/trailing whitespace or non-digit characters at start
            if(!std::isdigit(static_cast<unsigned char>(startStr[0])) ||
               !std::isdigit(static_cast<unsigned char>(endStr[0])))
            {
                std::cerr << "Invalid range format: " << sel << "\n";
                return std::nullopt;
            }

            std::size_t startPos = 0;
            std::size_t endPos = 0;
            const auto start = std::stoul(startStr, &startPos);
            const auto end = std::stoul(endStr, &endPos);

            // Ensure the entire string was consumed (no trailing characters)
            if(startPos != startStr.length() || endPos != endStr.length())
            {
                std::cerr << "Invalid range format: " << sel << "\n";
                return std::nullopt;
            }

            if(start >= 1 && start <= end)
            {
                return std::make_pair(start, end);
            }
        }
        catch(const std::invalid_argument&)
        {
            std::cerr << "Invalid range format: " << sel << "\n";
        }
        catch(const std::out_of_range&)
        {
            std::cerr << "Range values out of bounds: " << sel << "\n";
        }
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

    try
    {
        // Check for empty string or leading whitespace/non-digit
        if(sel.empty() || !std::isdigit(static_cast<unsigned char>(sel[0])))
        {
            std::cerr << "Invalid index provided: " << sel << "\n";
            return std::nullopt;
        }

        std::size_t pos = 0;
        const auto index = std::stoul(sel, &pos);

        // Ensure the entire string was consumed (no trailing characters)
        if(pos != sel.length())
        {
            std::cerr << "Invalid index provided: " << sel << "\n";
            return std::nullopt;
        }

        if(index >= 1 && index <= maxIndex)
        {
            return {{index - 1}};
        }
        std::cerr << "Index out of valid range: " << sel << "\n";
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

