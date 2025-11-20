// filepath: /Users/simone/dev/sandbox/pointclouds/STEPToPoints/tests/solid_index_parser_tests.cpp

#include <gtest/gtest.h>
#include "solid_index_parser.hpp"
#include <vector>
#include <string>

// ============================================================================
// Tests for generateRange()
// ============================================================================

TEST(GenerateRangeTest, SingleElement)
{
    const auto result = generateRange(1, 1);
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], 0); // 1-based to 0-based
}

TEST(GenerateRangeTest, SmallRange)
{
    const auto result = generateRange(1, 5);
    const std::vector<std::size_t> expected{0, 1, 2, 3, 4};
    EXPECT_EQ(result, expected);
}

TEST(GenerateRangeTest, LargeRange)
{
    const auto result = generateRange(10, 20);
    ASSERT_EQ(result.size(), 11);
    EXPECT_EQ(result.front(), 9);  // 10 - 1
    EXPECT_EQ(result.back(), 19);  // 20 - 1
}

TEST(GenerateRangeTest, MiddleRange)
{
    const auto result = generateRange(5, 7);
    const std::vector<std::size_t> expected{4, 5, 6};
    EXPECT_EQ(result, expected);
}

TEST(GenerateRangeTest, ConsecutiveIndices)
{
    const auto result = generateRange(100, 101);
    const std::vector<std::size_t> expected{99, 100};
    EXPECT_EQ(result, expected);
}

// ============================================================================
// Tests for parseRange()
// ============================================================================

struct ParseRangeTestCase
{
    std::string input;
    bool expectSuccess;
    std::size_t expectedStart;
    std::size_t expectedEnd;
    std::string description;
};

class ParseRangeParameterizedTest : public ::testing::TestWithParam<ParseRangeTestCase>
{
};

TEST_P(ParseRangeParameterizedTest, ParseRangeVariousInputs)
{
    const auto& testCase = GetParam();
    const auto result = parseRange(testCase.input);

    if(testCase.expectSuccess)
    {
        ASSERT_TRUE(result.has_value()) << "Failed for: " << testCase.description;
        EXPECT_EQ(result->first, testCase.expectedStart) << testCase.description;
        EXPECT_EQ(result->second, testCase.expectedEnd) << testCase.description;
    }
    else
    {
        EXPECT_FALSE(result.has_value()) << "Should fail for: " << testCase.description;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ValidRanges,
    ParseRangeParameterizedTest,
    ::testing::Values(
        ParseRangeTestCase{"1-5", true, 1, 5, "Simple range"},
        ParseRangeTestCase{"1-1", true, 1, 1, "Single element range"},
        ParseRangeTestCase{"10-20", true, 10, 20, "Two digit numbers"},
        ParseRangeTestCase{"100-200", true, 100, 200, "Three digit numbers"},
        ParseRangeTestCase{"1-1000", true, 1, 1000, "Large range"},
        ParseRangeTestCase{"999-1001", true, 999, 1001, "Large indices"}
    )
);

INSTANTIATE_TEST_SUITE_P(
    InvalidRanges,
    ParseRangeParameterizedTest,
    ::testing::Values(
        ParseRangeTestCase{"5-3", false, 0, 0, "End before start"},
        ParseRangeTestCase{"0-5", false, 0, 0, "Start is zero"},
        ParseRangeTestCase{"0-0", false, 0, 0, "Both zero"},
        ParseRangeTestCase{"-5", false, 0, 0, "Missing start"},
        ParseRangeTestCase{"5-", false, 0, 0, "Missing end"},
        ParseRangeTestCase{"-", false, 0, 0, "Only dash"},
        ParseRangeTestCase{"--", false, 0, 0, "Double dash"},
        ParseRangeTestCase{"a-5", false, 0, 0, "Non-numeric start"},
        ParseRangeTestCase{"5-b", false, 0, 0, "Non-numeric end"},
        ParseRangeTestCase{"a-b", false, 0, 0, "Both non-numeric"},
        ParseRangeTestCase{"1.5-3.5", false, 0, 0, "Decimal numbers"},
        ParseRangeTestCase{"1 - 5", false, 0, 0, "Spaces in range"},
        ParseRangeTestCase{"", false, 0, 0, "Empty string"},
        ParseRangeTestCase{"5", false, 0, 0, "Single number without dash"},
        ParseRangeTestCase{"1-2-3", false, 0, 0, "Multiple dashes"},
        ParseRangeTestCase{"-1-5", false, 0, 0, "Negative start"},
        ParseRangeTestCase{"1--5", false, 0, 0, "Double dash in middle"}
    )
);

TEST(ParseRangeTest, VeryLargeNumbers)
{
    // Test with numbers that might overflow
    const auto result = parseRange("999999999999999999999-999999999999999999999");
    EXPECT_FALSE(result.has_value()); // Should fail due to out_of_range
}

// ============================================================================
// Tests for parseSolidIndex()
// ============================================================================

struct ParseSolidIndexTestCase
{
    std::string input;
    std::size_t maxIndex;
    bool expectSuccess;
    std::vector<std::size_t> expectedIndices;
    std::string description;
};

class ParseSolidIndexParameterizedTest : public ::testing::TestWithParam<ParseSolidIndexTestCase>
{
};

TEST_P(ParseSolidIndexParameterizedTest, ParseSolidIndexVariousInputs)
{
    const auto& testCase = GetParam();
    const auto result = parseSolidIndex(testCase.input, testCase.maxIndex);

    if(testCase.expectSuccess)
    {
        ASSERT_TRUE(result.has_value()) << "Failed for: " << testCase.description;
        EXPECT_EQ(*result, testCase.expectedIndices) << testCase.description;
    }
    else
    {
        EXPECT_FALSE(result.has_value()) << "Should fail for: " << testCase.description;
    }
}

INSTANTIATE_TEST_SUITE_P(
    ValidSingleIndices,
    ParseSolidIndexParameterizedTest,
    ::testing::Values(
        ParseSolidIndexTestCase{"1", 10, true, {0}, "First index"},
        ParseSolidIndexTestCase{"5", 10, true, {4}, "Middle index"},
        ParseSolidIndexTestCase{"10", 10, true, {9}, "Last index"},
        ParseSolidIndexTestCase{"1", 1, true, {0}, "Only one element available"},
        ParseSolidIndexTestCase{"100", 100, true, {99}, "Large single index"}
    )
);

INSTANTIATE_TEST_SUITE_P(
    ValidRangeIndices,
    ParseSolidIndexParameterizedTest,
    ::testing::Values(
        ParseSolidIndexTestCase{"1-5", 10, true, {0, 1, 2, 3, 4}, "Simple range"},
        ParseSolidIndexTestCase{"1-1", 10, true, {0}, "Single element range"},
        ParseSolidIndexTestCase{"8-10", 10, true, {7, 8, 9}, "Range at end"},
        ParseSolidIndexTestCase{"1-10", 10, true, {0, 1, 2, 3, 4, 5, 6, 7, 8, 9}, "Full range"},
        ParseSolidIndexTestCase{"5-7", 20, true, {4, 5, 6}, "Range in middle"}
    )
);

INSTANTIATE_TEST_SUITE_P(
    InvalidIndices,
    ParseSolidIndexParameterizedTest,
    ::testing::Values(
        ParseSolidIndexTestCase{"0", 10, false, {}, "Zero index"},
        ParseSolidIndexTestCase{"11", 10, false, {}, "Index exceeds max"},
        ParseSolidIndexTestCase{"100", 10, false, {}, "Index far exceeds max"},
        ParseSolidIndexTestCase{"-1", 10, false, {}, "Negative index"},
        ParseSolidIndexTestCase{"", 10, false, {}, "Empty string"},
        ParseSolidIndexTestCase{"abc", 10, false, {}, "Non-numeric string"},
        ParseSolidIndexTestCase{"1.5", 10, false, {}, "Decimal number"},
        ParseSolidIndexTestCase{"1 5", 10, false, {}, "Space in number"},
        ParseSolidIndexTestCase{" 5", 10, false, {}, "Leading space"},
        ParseSolidIndexTestCase{"5 ", 10, false, {}, "Trailing space"},
        ParseSolidIndexTestCase{"1,5", 10, false, {}, "Comma separator"},
        ParseSolidIndexTestCase{"1+5", 10, false, {}, "Plus sign"}
    )
);

INSTANTIATE_TEST_SUITE_P(
    InvalidRangeIndices,
    ParseSolidIndexParameterizedTest,
    ::testing::Values(
        ParseSolidIndexTestCase{"1-15", 10, false, {}, "Range end exceeds max"},
        ParseSolidIndexTestCase{"5-3", 10, false, {}, "Range end before start"},
        ParseSolidIndexTestCase{"0-5", 10, false, {}, "Range starts at zero"},
        ParseSolidIndexTestCase{"11-15", 10, false, {}, "Range completely out of bounds"},
        ParseSolidIndexTestCase{"-5", 10, false, {}, "Missing range start"},
        ParseSolidIndexTestCase{"5-", 10, false, {}, "Missing range end"},
        ParseSolidIndexTestCase{"-", 10, false, {}, "Only dash"},
        ParseSolidIndexTestCase{"a-5", 10, false, {}, "Non-numeric range start"},
        ParseSolidIndexTestCase{"5-b", 10, false, {}, "Non-numeric range end"}
    )
);

TEST(ParseSolidIndexTest, MaxIndexZero)
{
    // Edge case: no solids available
    const auto result = parseSolidIndex("1", 0);
    EXPECT_FALSE(result.has_value());
}

TEST(ParseSolidIndexTest, LargeValidRange)
{
    const auto result = parseSolidIndex("1-100", 100);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->size(), 100);
    EXPECT_EQ(result->front(), 0);
    EXPECT_EQ(result->back(), 99);
}

TEST(ParseSolidIndexTest, ConsecutiveCalls)
{
    // Test that function doesn't maintain state between calls
    const auto result1 = parseSolidIndex("1", 10);
    const auto result2 = parseSolidIndex("5", 10);
    const auto result3 = parseSolidIndex("1-3", 10);

    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(result2.has_value());
    ASSERT_TRUE(result3.has_value());

    EXPECT_EQ(*result1, std::vector<std::size_t>{0});
    EXPECT_EQ(*result2, std::vector<std::size_t>{4});
    EXPECT_EQ(*result3, std::vector<std::size_t>({0, 1, 2}));
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(SolidIndexParserIntegrationTest, RangeToIndexConversion)
{
    // Test that parseRange -> generateRange -> matches parseSolidIndex
    const std::string input = "5-10";
    const std::size_t maxIndex = 20;

    const auto rangeOpt = parseRange(input);
    ASSERT_TRUE(rangeOpt.has_value());

    const auto [start, end] = *rangeOpt;
    const auto generatedIndices = generateRange(start, end);

    const auto parsedIndices = parseSolidIndex(input, maxIndex);
    ASSERT_TRUE(parsedIndices.has_value());

    EXPECT_EQ(generatedIndices, *parsedIndices);
}

TEST(SolidIndexParserIntegrationTest, BoundaryConditions)
{
    struct BoundaryTest
    {
        std::string input;
        std::size_t maxIndex;
        bool shouldSucceed;
    };

    const std::vector<BoundaryTest> tests = {
        {"1", 1, true},           // Minimum valid case
        {"1", 2, true},           // First of multiple
        {"2", 2, true},           // Last of multiple
        {"3", 2, false},          // Just over boundary
        {"1-1", 1, true},         // Single element range
        {"1-2", 2, true},         // Full range
        {"1-3", 2, false},        // Range exceeds max
        {"2-2", 2, true},         // Range at boundary
        {"2-3", 2, false},        // Range starts at boundary, exceeds
    };

    for(const auto& test : tests)
    {
        const auto result = parseSolidIndex(test.input, test.maxIndex);
        if(test.shouldSucceed)
        {
            EXPECT_TRUE(result.has_value())
                << "Failed for input: " << test.input << " maxIndex: " << test.maxIndex;
        }
        else
        {
            EXPECT_FALSE(result.has_value())
                << "Should have failed for input: " << test.input << " maxIndex: " << test.maxIndex;
        }
    }
}

TEST(SolidIndexParserIntegrationTest, ResultSizeConsistency)
{
    struct SizeTest
    {
        std::string input;
        std::size_t maxIndex;
        std::size_t expectedSize;
    };

    const std::vector<SizeTest> tests = {
        {"1", 10, 1},
        {"5", 10, 1},
        {"1-1", 10, 1},
        {"1-5", 10, 5},
        {"1-10", 10, 10},
        {"5-9", 10, 5},
        {"8-10", 10, 3},
    };

    for(const auto& test : tests)
    {
        const auto result = parseSolidIndex(test.input, test.maxIndex);
        ASSERT_TRUE(result.has_value()) << "Failed for: " << test.input;
        EXPECT_EQ(result->size(), test.expectedSize)
            << "Size mismatch for input: " << test.input;
    }
}

