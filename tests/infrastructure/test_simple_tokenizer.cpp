/**
 * @file test_simple_tokenizer.cpp
 * @brief Unit tests for SimpleTokenizer
 */

#include <gtest/gtest.h>

#include "infrastructure/ai/simple_tokenizer.h"

using namespace fmf;

// Test: Basic tokenization
TEST(SimpleTokenizerTest, BasicTokenization)
{
    SimpleTokenizer tokenizer;
    auto tokens = tokenizer.tokenize("Hello world");

    // Should have [CLS], token1, token2, [SEP]
    EXPECT_GE(tokens.size(), 4);
    EXPECT_EQ(tokens.front(), SimpleTokenizer::CLS_TOKEN_ID);  // [CLS]
    EXPECT_EQ(tokens.back(), SimpleTokenizer::SEP_TOKEN_ID);   // [SEP]
}

// Test: Empty text handling
TEST(SimpleTokenizerTest, EmptyText)
{
    SimpleTokenizer tokenizer;
    auto tokens = tokenizer.tokenize("");

    // Should have at least [CLS] and [SEP]
    EXPECT_EQ(tokens.size(), 2);
    EXPECT_EQ(tokens[0], SimpleTokenizer::CLS_TOKEN_ID);
    EXPECT_EQ(tokens[1], SimpleTokenizer::SEP_TOKEN_ID);
}

// Test: Lowercase conversion
TEST(SimpleTokenizerTest, LowercaseConversion)
{
    SimpleTokenizer tokenizer(512, true);  // doLowerCase=true
    auto tokens1 = tokenizer.tokenize("Hello");
    auto tokens2 = tokenizer.tokenize("hello");

    // Same text (ignoring case) should produce same tokens
    ASSERT_EQ(tokens1.size(), tokens2.size());
    for (size_t i = 0; i < tokens1.size(); ++i)
    {
        EXPECT_EQ(tokens1[i], tokens2[i]);
    }
}

// Test: Case sensitivity option
TEST(SimpleTokenizerTest, CaseSensitive)
{
    SimpleTokenizer tokenizer(512, false);  // doLowerCase=false
    auto tokens1 = tokenizer.tokenize("Hello");
    auto tokens2 = tokenizer.tokenize("hello");

    // Different case should produce different tokens (except special tokens)
    bool foundDifference = false;
    for (size_t i = 1; i < tokens1.size() - 1; ++i)
    {
        if (tokens1[i] != tokens2[i])
        {
            foundDifference = true;
            break;
        }
    }
    EXPECT_TRUE(foundDifference);
}

// Test: Punctuation handling
TEST(SimpleTokenizerTest, PunctuationStripping)
{
    SimpleTokenizer tokenizer;
    auto tokens1 = tokenizer.tokenize("hello");
    auto tokens2 = tokenizer.tokenize("hello!");
    auto tokens3 = tokenizer.tokenize("(hello)");

    // Punctuation should be stripped, same word tokens
    EXPECT_GE(tokens1.size(), 3);
    EXPECT_GE(tokens2.size(), 3);
    EXPECT_GE(tokens3.size(), 3);

    // Middle tokens (word tokens) should be the same
    EXPECT_EQ(tokens1[1], tokens2[1]);
    EXPECT_EQ(tokens1[1], tokens3[1]);
}

// Test: Multiple words
TEST(SimpleTokenizerTest, MultipleWords)
{
    SimpleTokenizer tokenizer;
    auto tokens = tokenizer.tokenize("The quick brown fox");

    // [CLS] + 4 words + [SEP] = 6 tokens
    EXPECT_EQ(tokens.size(), 6);
    EXPECT_EQ(tokens[0], SimpleTokenizer::CLS_TOKEN_ID);
    EXPECT_EQ(tokens[5], SimpleTokenizer::SEP_TOKEN_ID);

    // All middle tokens should be different (unlikely hash collision)
    EXPECT_NE(tokens[1], tokens[2]);
    EXPECT_NE(tokens[2], tokens[3]);
    EXPECT_NE(tokens[3], tokens[4]);
}

// Test: Max length truncation
TEST(SimpleTokenizerTest, MaxLengthTruncation)
{
    SimpleTokenizer tokenizer(10);  // Max length = 10

    // Generate long text (more than 10 words)
    std::string longText;
    for (int i = 0; i < 20; ++i)
    {
        longText += "word" + std::to_string(i) + " ";
    }

    auto tokens = tokenizer.tokenize(longText);

    // Should be truncated to max length
    EXPECT_LE(tokens.size(), 10);
    EXPECT_EQ(tokens.front(), SimpleTokenizer::CLS_TOKEN_ID);
    // Last token might be [SEP] or a word token (if truncated)
}

// Test: Special tokens are always present
TEST(SimpleTokenizerTest, SpecialTokensPresent)
{
    SimpleTokenizer tokenizer;
    auto tokens = tokenizer.tokenize("test");

    bool hasCLS = (tokens.front() == SimpleTokenizer::CLS_TOKEN_ID);
    bool hasSEP = (tokens.back() == SimpleTokenizer::SEP_TOKEN_ID);

    EXPECT_TRUE(hasCLS);
    EXPECT_TRUE(hasSEP);
}

// Test: Deterministic behavior
TEST(SimpleTokenizerTest, DeterministicTokenization)
{
    SimpleTokenizer tokenizer;
    std::string text = "Hello world this is a test";

    auto tokens1 = tokenizer.tokenize(text);
    auto tokens2 = tokenizer.tokenize(text);

    ASSERT_EQ(tokens1.size(), tokens2.size());
    for (size_t i = 0; i < tokens1.size(); ++i)
    {
        EXPECT_EQ(tokens1[i], tokens2[i]);
    }
}

// Test: Whitespace handling
TEST(SimpleTokenizerTest, MultipleWhitespaces)
{
    SimpleTokenizer tokenizer;
    auto tokens1 = tokenizer.tokenize("hello world");
    auto tokens2 = tokenizer.tokenize("hello    world");     // Multiple spaces
    auto tokens3 = tokenizer.tokenize("  hello   world  ");  // Leading/trailing

    // Same words should produce same tokens
    EXPECT_EQ(tokens1.size(), tokens2.size());
    EXPECT_EQ(tokens1.size(), tokens3.size());

    for (size_t i = 0; i < tokens1.size(); ++i)
    {
        EXPECT_EQ(tokens1[i], tokens2[i]);
        EXPECT_EQ(tokens1[i], tokens3[i]);
    }
}

// Test: Token IDs are in valid range
TEST(SimpleTokenizerTest, TokenIDRange)
{
    SimpleTokenizer tokenizer;
    auto tokens = tokenizer.tokenize(
        "The quick brown fox jumps over the lazy "
        "dog");

    for (auto token : tokens)
    {
        // Token IDs should be non-negative and within vocab size
        EXPECT_GE(token, 0);
        EXPECT_LT(token, SimpleTokenizer::DEFAULT_VOCAB_SIZE);
    }
}

// Test: Special characters
TEST(SimpleTokenizerTest, SpecialCharacters)
{
    SimpleTokenizer tokenizer;

    // Should handle special characters gracefully
    EXPECT_NO_THROW({
        tokenizer.tokenize("hello@world.com");
        tokenizer.tokenize("c++");
        tokenizer.tokenize("hello-world");
        tokenizer.tokenize("50%");
    });
}

// Test: getMaxLength accessor
TEST(SimpleTokenizerTest, GetMaxLength)
{
    SimpleTokenizer tokenizer1(256);
    SimpleTokenizer tokenizer2(512);

    EXPECT_EQ(tokenizer1.getMaxLength(), 256);
    EXPECT_EQ(tokenizer2.getMaxLength(), 512);
}
