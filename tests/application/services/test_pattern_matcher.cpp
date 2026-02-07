#include <gtest/gtest.h>

#include "application/services/pattern_matcher.h"

using namespace fmf;

class PatternMatcherTest : public ::testing::Test
{
};

TEST_F(PatternMatcherTest, ExactMatchCaseSensitive)
{
    EXPECT_TRUE(PatternMatcher::matchExact("hello", "hello", true));
    EXPECT_FALSE(PatternMatcher::matchExact("hello", "Hello", true));
    EXPECT_FALSE(PatternMatcher::matchExact("hello", "world", true));
}

TEST_F(PatternMatcherTest, ExactMatchCaseInsensitive)
{
    EXPECT_TRUE(PatternMatcher::matchExact("hello", "hello", false));
    EXPECT_TRUE(PatternMatcher::matchExact("hello", "Hello", false));
    EXPECT_TRUE(PatternMatcher::matchExact("HELLO", "hello", false));
    EXPECT_FALSE(PatternMatcher::matchExact("hello", "world", false));
}

TEST_F(PatternMatcherTest, WildcardStar)
{
    // * matches any sequence
    EXPECT_TRUE(PatternMatcher::matchWildcard("*.cpp", "test.cpp", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("test*", "test.cpp", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("*test*", "my_test_file", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("*", "anything", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*c", "abc", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*c", "ac", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*c", "abbbbc", true));

    EXPECT_FALSE(PatternMatcher::matchWildcard("*.cpp", "test.h", true));
    EXPECT_FALSE(PatternMatcher::matchWildcard("test*", "mytest", true));
}

TEST_F(PatternMatcherTest, WildcardQuestion)
{
    // ? matches exactly one character
    EXPECT_TRUE(PatternMatcher::matchWildcard("test?.cpp", "test1.cpp", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("test?.cpp", "testa.cpp", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("?est", "test", true));

    EXPECT_FALSE(PatternMatcher::matchWildcard("test?.cpp", "test.cpp", true));
    EXPECT_FALSE(
        PatternMatcher::matchWildcard("test?.cpp", "test12.cpp", true));
}

TEST_F(PatternMatcherTest, WildcardCombined)
{
    // Combine * and ?
    EXPECT_TRUE(PatternMatcher::matchWildcard("*.??", "file.cc", true));
    EXPECT_TRUE(
        PatternMatcher::matchWildcard("test_*.c?p", "test_file.cpp", true));
    EXPECT_TRUE(
        PatternMatcher::matchWildcard("*test?.txt", "my_test1.txt", true));

    EXPECT_FALSE(PatternMatcher::matchWildcard("*.??", "file.cpp", true));
}

TEST_F(PatternMatcherTest, WildcardCaseInsensitive)
{
    EXPECT_TRUE(PatternMatcher::matchWildcard("*.CPP", "test.cpp", false));
    EXPECT_TRUE(PatternMatcher::matchWildcard("TEST*", "test.cpp", false));
    EXPECT_TRUE(PatternMatcher::matchWildcard("*TeSt*", "my_test_file", false));
}

TEST_F(PatternMatcherTest, EmptyPattern)
{
    EXPECT_TRUE(PatternMatcher::matchWildcard("", "", true));
    EXPECT_FALSE(PatternMatcher::matchWildcard("", "test", true));
}

TEST_F(PatternMatcherTest, ComplexPatterns)
{
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*b*c", "aXbYc", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*b*c", "abc", true));
    EXPECT_TRUE(PatternMatcher::matchWildcard("a*b*c", "aXXXbYYYc", true));

    EXPECT_FALSE(PatternMatcher::matchWildcard("a*b*c", "acb", true));
}
