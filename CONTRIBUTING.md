# Contributing to Find My Files

First off, thank you for considering contributing to Find My Files! It's people like you that make this tool better for everyone.

## Code of Conduct

This project and everyone participating in it is governed by our Code of Conduct. By participating, you are expected to uphold this code. Please report unacceptable behavior to the project maintainers.

## How Can I Contribute?

### Reporting Bugs

Before creating bug reports, please check the issue list as you might find out that you don't need to create one. When you are creating a bug report, please include as many details as possible:

* **Use a clear and descriptive title** for the issue to identify the problem
* **Describe the exact steps which reproduce the problem** in as many details as possible
* **Provide specific examples** to demonstrate the steps
* **Describe the behavior you observed** after following the steps
* **Explain which behavior you expected to see instead** and why
* **Include screenshots or animated GIFs** if applicable
* **Include your environment details** (OS, compiler version, CMake version)

### Suggesting Enhancements

Enhancement suggestions are tracked as GitHub issues. When creating an enhancement suggestion, please include:

* **Use a clear and descriptive title** for the issue
* **Provide a step-by-step description** of the suggested enhancement
* **Provide specific examples** to demonstrate the steps or point out the part where the suggestion is related to
* **Describe the current behavior** and **explain which behavior you expected to see instead**
* **Explain why this enhancement would be useful** to most users

### Pull Requests

* Fill in the required template
* Follow the C++ coding style guide (see below)
* Include appropriate test cases
* Update documentation as needed
* End all files with a newline

## Development Process

### Setting Up Your Development Environment

```bash
# Clone your fork of the repo
git clone https://github.com/YOUR-USERNAME/find_my_file.git
cd find_my_file

# Add upstream remote
git remote add upstream https://github.com/leeh8911/find_my_file.git

# Create a branch for your changes
git checkout -b feature/your-feature-name
```

### Building the Project

```bash
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Running Tests

```bash
# Run all unit tests
./tests

# Run specific test suite
./tests --gtest_filter="FileScannerTest.*"

# Run integration tests
cd ../test/integrationtest
./run_all.sh
```

### Code Style

We follow the Google C++ Style Guide with some modifications:

#### Formatting

* Use `clang-format` for automatic formatting
* Configuration is provided in `.clang-format`
* Run before committing: `clang-format -i src/*.cpp include/*.h`

#### Naming Conventions

* Classes: `PascalCase` (e.g., `FileScanner`, `SearchCriteria`)
* Functions/Methods: `camelCase` (e.g., `scanDirectory`, `matchPattern`)
* Variables: `camelCase` with trailing underscore for private members (e.g., `fileCount_`)
* Constants: `kConstantName` (e.g., `kMaxFileSize`)
* Namespaces: `lowercase` (e.g., `fmf`)

#### Comments

* Use `///` or `/** */` for documentation comments
* Follow Doxygen style for API documentation
* Include `@brief`, `@param`, `@return`, `@throws` as appropriate

#### Example

```cpp
/**
 * @brief Scans a directory for files matching criteria
 * 
 * @param dirPath Path to directory to scan
 * @param recursive Whether to scan recursively
 * @param criteria Search criteria to apply
 * @return SearchResult containing matched files
 * @throws std::filesystem::filesystem_error on access errors
 */
SearchResult scanDirectory(const std::filesystem::path& dirPath,
                          bool recursive,
                          const SearchCriteria& criteria);
```

### Git Commit Messages

* Use the present tense ("Add feature" not "Added feature")
* Use the imperative mood ("Move cursor to..." not "Moves cursor to...")
* Limit the first line to 72 characters or less
* Reference issues and pull requests liberally after the first line
* Consider starting the commit message with an applicable emoji:
    * 🎨 `:art:` when improving the format/structure of the code
    * 🐎 `:racehorse:` when improving performance
    * 📝 `:memo:` when writing docs
    * 🐛 `:bug:` when fixing a bug
    * 🔥 `:fire:` when removing code or files
    * ✅ `:white_check_mark:` when adding tests
    * 🔒 `:lock:` when dealing with security

### Commit Message Format

```
type: brief description (max 72 characters)

More detailed explanatory text, if necessary. Wrap it to about 72
characters or so. In some contexts, the first line is treated as the
subject of an email and the rest of the text as the body.

- Bullet points are okay, too
- Use a hyphen or asterisk for the bullet

Resolves: #123
See also: #456, #789
```

Types:
* `feat`: New feature
* `fix`: Bug fix
* `docs`: Documentation only changes
* `style`: Changes that don't affect code meaning (formatting, etc.)
* `refactor`: Code change that neither fixes a bug nor adds a feature
* `perf`: Performance improvement
* `test`: Adding or updating tests
* `build`: Changes to build system or dependencies
* `ci`: Changes to CI configuration
* `chore`: Other changes that don't modify src or test files

## Testing Guidelines

### Unit Tests

* Write tests for all new functionality
* Use Google Test framework
* Follow AAA pattern (Arrange, Act, Assert)
* Test edge cases and error conditions
* Keep tests focused and independent

Example:
```cpp
TEST_F(FileScannerTest, ScanEmptyDirectory)
{
    // Arrange
    SearchResult result;
    FileScanner scanner;
    
    // Act
    result = scanner.scanDirectory(emptyDirPath_);
    
    // Assert
    EXPECT_EQ(result.getFiles().size(), 0);
}
```

### Integration Tests

* Write bash scripts for end-to-end scenarios
* Place in `test/integrationtest/`
* Name files `uc_*.sh` (use case)
* Include clear pass/fail output

## Documentation

* Update relevant documentation (README.md, docs/)
* Add docstrings for new classes and methods
* Include usage examples for new features
* Update architecture.md for significant design changes

## Review Process

1. Ensure all tests pass
2. Update documentation
3. Submit pull request
4. Address review comments
5. Maintainer will merge when approved

## Questions?

Feel free to ask questions by:
* Opening an issue with the "question" label
* Starting a discussion on GitHub Discussions
* Contacting the maintainers

Thank you for contributing! 🎉
