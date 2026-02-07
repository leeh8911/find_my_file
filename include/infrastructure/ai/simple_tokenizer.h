/**
 * @file simple_tokenizer.h
 * @brief Simple text tokenizer for LocalEmbeddingProvider
 *
 * Provides basic tokenization functionality.
 * TODO: Replace with proper BPE/WordPiece tokenizer for production.
 */

#ifndef FMF_SIMPLE_TOKENIZER_H
#define FMF_SIMPLE_TOKENIZER_H

#pragma once

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fmf
{

/**
 * @brief Simple whitespace-based tokenizer
 *
 * Features:
 * - Lowercase conversion
 * - Punctuation handling
 * - Special tokens ([CLS], [SEP], [PAD], [UNK])
 * - Max sequence length (default: 512)
 *
 * Limitations:
 * - Not subword-based (no BPE/WordPiece)
 * - Simple vocabulary (hash-based)
 * - Not compatible with real BERT models
 *
 * TODO: Implement proper BPE/WordPiece tokenizer
 */
class SimpleTokenizer
{
 public:
    // Special token IDs
    static constexpr int64_t PAD_TOKEN_ID = 0;
    static constexpr int64_t UNK_TOKEN_ID = 100;
    static constexpr int64_t CLS_TOKEN_ID = 101;
    static constexpr int64_t SEP_TOKEN_ID = 102;

    // Default settings
    static constexpr size_t DEFAULT_MAX_LENGTH = 512;
    static constexpr size_t DEFAULT_VOCAB_SIZE = 30000;

    /**
     * @brief Construct tokenizer
     *
     * @param maxLength Maximum sequence length (default: 512)
     * @param doLowerCase Convert to lowercase (default: true)
     */
    explicit SimpleTokenizer(size_t maxLength = DEFAULT_MAX_LENGTH,
                             bool doLowerCase = true)
        : m_maxLength(maxLength), m_doLowerCase(doLowerCase)
    {
    }

    /**
     * @brief Tokenize text to token IDs
     *
     * Process:
     * 1. Lowercase (if enabled)
     * 2. Split by whitespace
     * 3. Convert words to IDs (hash-based)
     * 4. Add special tokens ([CLS], [SEP])
     * 5. Truncate/pad to max_length
     *
     * @param text Input text
     * @return Vector of token IDs
     */
    std::vector<int64_t> tokenize(const std::string& text) const
    {
        std::vector<int64_t> tokens;

        // Add [CLS] token
        tokens.push_back(CLS_TOKEN_ID);

        // Preprocess text
        std::string processed = preprocess(text);

        // Split by whitespace
        std::istringstream stream(processed);
        std::string word;

        while (stream >> word && tokens.size() < m_maxLength - 1)
        {
            // Remove punctuation from ends
            word = stripPunctuation(word);

            if (!word.empty())
            {
                tokens.push_back(wordToId(word));
            }
        }

        // Add [SEP] token
        if (tokens.size() < m_maxLength)
        {
            tokens.push_back(SEP_TOKEN_ID);
        }

        return tokens;
    }

    /**
     * @brief Get maximum sequence length
     */
    size_t getMaxLength() const { return m_maxLength; }

 private:
    /**
     * @brief Preprocess text (lowercase, etc.)
     */
    std::string preprocess(const std::string& text) const
    {
        if (!m_doLowerCase)
        {
            return text;
        }

        std::string result = text;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /**
     * @brief Remove leading/trailing punctuation
     */
    std::string stripPunctuation(const std::string& word) const
    {
        if (word.empty()) return word;

        size_t start = 0;
        size_t end = word.length();

        // Remove leading punctuation
        while (start < end && std::ispunct(word[start]))
        {
            ++start;
        }

        // Remove trailing punctuation
        while (end > start && std::ispunct(word[end - 1]))
        {
            --end;
        }

        return word.substr(start, end - start);
    }

    /**
     * @brief Convert word to token ID (simple hash)
     *
     * TODO: Replace with proper vocabulary lookup
     */
    int64_t wordToId(const std::string& word) const
    {
        if (word.empty())
        {
            return UNK_TOKEN_ID;
        }

        // Simple hash function
        int64_t hash = 0;
        for (char c : word)
        {
            hash = hash * 31 + static_cast<int64_t>(c);
        }

        // Map to vocab range, avoiding special tokens (0-102)
        int64_t vocabId = (std::abs(hash) % (DEFAULT_VOCAB_SIZE - 103)) + 103;
        return vocabId;
    }

    size_t m_maxLength;
    bool m_doLowerCase;
};

}  // namespace fmf

#endif  // FMF_SIMPLE_TOKENIZER_H
