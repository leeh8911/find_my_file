#include <gtest/gtest.h>

#include "domain/value_objects/search_criteria.h"

using namespace fmf;

class SearchCriteriaTest : public ::testing::Test
{
};

TEST_F(SearchCriteriaTest, DefaultConstructor)
{
    SearchCriteria criteria;
    EXPECT_TRUE(criteria.isEmpty());
    EXPECT_TRUE(criteria.isCaseSensitive());
    EXPECT_FALSE(criteria.isFilesOnly());
    EXPECT_FALSE(criteria.isDirectoriesOnly());
}

TEST_F(SearchCriteriaTest, SetNamePattern)
{
    SearchCriteria criteria;
    criteria.setNamePattern("*.cpp");

    EXPECT_EQ(criteria.getNamePattern(), "*.cpp");
    EXPECT_FALSE(criteria.isEmpty());
}

TEST_F(SearchCriteriaTest, SetExtensions)
{
    SearchCriteria criteria;
    criteria.addExtension(".cpp");
    criteria.addExtension("h");  // Should add dot automatically

    EXPECT_TRUE(criteria.hasExtensions());
    EXPECT_EQ(criteria.getExtensions().size(), 2);
    EXPECT_EQ(criteria.getExtensions()[0], ".cpp");
    EXPECT_EQ(criteria.getExtensions()[1], ".h");
}

TEST_F(SearchCriteriaTest, SetExtensionsVector)
{
    SearchCriteria criteria;
    std::vector<std::string> exts = {"cpp", ".h", ".cc"};
    criteria.setExtensions(exts);

    EXPECT_TRUE(criteria.hasExtensions());
    EXPECT_EQ(criteria.getExtensions().size(), 3);
    // All should have dots
    for (const auto& ext : criteria.getExtensions())
    {
        EXPECT_EQ(ext[0], '.');
    }
}

TEST_F(SearchCriteriaTest, CaseSensitivity)
{
    SearchCriteria criteria;
    EXPECT_TRUE(criteria.isCaseSensitive());

    criteria.setCaseSensitive(false);
    EXPECT_FALSE(criteria.isCaseSensitive());
}

TEST_F(SearchCriteriaTest, FileTypeFilters)
{
    SearchCriteria criteria;

    criteria.setFilesOnly(true);
    EXPECT_TRUE(criteria.isFilesOnly());
    EXPECT_FALSE(criteria.isDirectoriesOnly());

    criteria.setDirectoriesOnly(true);
    EXPECT_TRUE(criteria.isDirectoriesOnly());
}

TEST_F(SearchCriteriaTest, SizeFilters)
{
    SearchCriteria criteria;

    EXPECT_FALSE(criteria.getMinSize().has_value());
    EXPECT_FALSE(criteria.getMaxSize().has_value());

    criteria.setMinSize(1000);
    criteria.setMaxSize(5000);

    EXPECT_TRUE(criteria.getMinSize().has_value());
    EXPECT_EQ(criteria.getMinSize().value(), 1000);
    EXPECT_TRUE(criteria.getMaxSize().has_value());
    EXPECT_EQ(criteria.getMaxSize().value(), 5000);
}

TEST_F(SearchCriteriaTest, PathPattern)
{
    SearchCriteria criteria;
    criteria.setPathPattern("*/tests/*");

    EXPECT_EQ(criteria.getPathPattern(), "*/tests/*");
    EXPECT_FALSE(criteria.isEmpty());
}

TEST_F(SearchCriteriaTest, IsEmpty)
{
    SearchCriteria criteria;
    EXPECT_TRUE(criteria.isEmpty());

    criteria.setNamePattern("test");
    EXPECT_FALSE(criteria.isEmpty());
}
