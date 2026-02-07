# Find My Files (fmf)

A fast, flexible, and powerful file search tool written in C++17.

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Version](https://img.shields.io/badge/version-0.6.0-blue)]()
[![License](https://img.shields.io/badge/license-MIT-green)]()

## Features

- 🔍 **Powerful Search**: Search by file name, path, extension, size, and modification date
- 🎯 **Pattern Matching**: Support for wildcards (`*`, `?`) and regular expressions
- 📄 **Content Search**: Search within file contents with text or regex patterns
- 🚀 **Parallel Scanning**: Multi-threaded directory traversal for better performance
- 🎨 **Multiple Output Formats**: Default, detailed, and JSON output formats with color support
- 🚫 **Ignore Patterns**: `.gitignore` style pattern support to exclude files
- 📊 **Logging System**: Comprehensive logging with multiple verbosity levels
- 🔒 **Thread-Safe**: Robust concurrent operations with proper synchronization

## Table of Contents

- [Installation](#installation)
- [Quick Start](#quick-start)
- [Usage](#usage)
- [Configuration File](#configuration-file)
- [Examples](#examples)
- [Building from Source](#building-from-source)
- [Running Tests](#running-tests)
- [Architecture](#architecture)
- [Contributing](#contributing)
- [License](#license)

## Installation

### From Source

```bash
# Clone the repository
git clone https://github.com/leeh8911/find_my_file.git
cd find_my_file

# Build
mkdir -p build && cd build
cmake ..
make

# Install (optional)
sudo make install
```

### Requirements

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 or higher
- pthread library (Linux/Unix)

## Quick Start

```bash
# Search for all C++ files in current directory
./find_my_files -r -e .cpp -e .h .

# Search for files containing "TODO" in src/ directory
./find_my_files -r -c "TODO" src/

# Find large files (>1MB) with detailed output
./find_my_files -r --min-size 1048576 --format detailed .

# Search with regex pattern and save as JSON
./find_my_files -r -x -n "test_.*\.cpp" . --format json > results.json
```

## Usage

```
find_my_files [OPTIONS] <directory>
```

### Search Options

| Option | Description |
|--------|-------------|
| `-n, --name PATTERN` | Search by file name (supports wildcards `*` and `?`) |
| `-e, --ext EXT` | Filter by extension (e.g., `.cpp`, `.h`) |
| `-p, --path PATTERN` | Search by path pattern |
| `-i, --ignore-case` | Case-insensitive search |
| `-x, --regex` | Use regular expressions for pattern matching |
| `-c, --content TEXT` | Search within file contents |

### File Type Options

| Option | Description |
|--------|-------------|
| `-f, --files-only` | Show only files (no directories) |
| `-D, --dirs-only` | Show only directories |

### Size Filters

| Option | Description |
|--------|-------------|
| `--min-size SIZE` | Minimum file size in bytes |
| `--max-size SIZE` | Maximum file size in bytes |

### Traversal Options

| Option | Description |
|--------|-------------|
| `-r, --recursive` | Scan directories recursively |
| `-d, --max-depth N` | Maximum recursion depth (use with `-r`) |
| `-l, --follow-links` | Follow symbolic links |
| `--ignore FILE` | Use ignore patterns from file (`.gitignore` style) |
| `-j, --threads N` | Number of threads for parallel scanning (0=sequential) |

### Output Options

| Option | Description |
|--------|-------------|
| `--format FORMAT` | Output format: `default`, `detailed`, `json` |
| `--color` | Enable colored output |
| `--no-color` | Disable colored output |
| `-v, --verbose` | Increase verbosity (`-v` for INFO, `-v -v` for DEBUG) |
| `--log-file FILE` | Write logs to specified file |

### Other Options

| Option | Description |
|--------|-------------|
| `-h, --help` | Display help message |

## Configuration File

Find My Files supports configuration files to set default search options and save search profiles. The tool automatically looks for `.findmyfilesrc` in:

1. Current directory (`./.findmyfilesrc`)
2. Home directory (`~/.findmyfilesrc`)
3. XDG config directory (`~/.config/find_my_files/config`)

CLI arguments always override settings from configuration files.

### Configuration Format

Configuration files use INI-style format with sections and key-value pairs:

```ini
# Default settings for all searches
[default]
recursive=true
follow_links=false
threads=4
format=detailed
color=true
verbosity=1
ignore_file=.gitignore

# Saved search profile for C++ files
[search.cpp_files]
extensions=.cpp,.h,.hpp
use_regex=false
files_only=true

# Saved search profile for TODO comments
[search.todos]
content=TODO|FIXME
use_regex=true
recursive=true
```

### Configuration Options

All CLI options can be specified in the `[default]` section:

| Key | Type | Description | Example |
|-----|------|-------------|---------|
| `recursive` | boolean | Enable recursive search | `recursive=true` |
| `follow_links` | boolean | Follow symbolic links | `follow_links=false` |
| `max_depth` | integer | Maximum recursion depth | `max_depth=5` |
| `threads` | integer | Number of threads | `threads=4` |
| `format` | string | Output format | `format=detailed` |
| `color` | boolean | Enable colored output | `color=true` |
| `verbosity` | integer | Verbosity level (0-2) | `verbosity=1` |
| `ignore_file` | string | Ignore patterns file | `ignore_file=.gitignore` |
| `log_file` | string | Log file path | `log_file=search.log` |
| `name` | string | Name pattern | `name=*.cpp` |
| `extensions` | string | Comma-separated extensions | `extensions=.c,.h` |
| `path` | string | Path pattern | `path=src/*` |
| `content` | string | Content search pattern | `content=TODO` |
| `use_regex` | boolean | Use regex patterns | `use_regex=true` |
| `ignore_case` | boolean | Case-insensitive search | `ignore_case=true` |
| `files_only` | boolean | Files only | `files_only=true` |
| `directories_only` | boolean | Directories only | `directories_only=false` |
| `min_size` | integer | Minimum file size (bytes) | `min_size=1024` |
| `max_size` | integer | Maximum file size (bytes) | `max_size=1048576` |

Boolean values accept: `true`, `yes`, `on`, `1` (for true) and `false`, `no`, `off`, `0` (for false).

### Example Configuration

See [examples/.findmyfilesrc](examples/.findmyfilesrc) for a complete example with multiple search profiles.

### Configuration Precedence

When the same option is specified in multiple places:

1. **CLI arguments** (highest priority) - Always override everything
2. **Config file** - Used as defaults
3. **Built-in defaults** (lowest priority) - Used when neither CLI nor config file specify a value

Example:
```bash
# Config file has: recursive=true
# This search will NOT be recursive (CLI overrides config)
./find_my_files . --no-recursive
```

## Examples

### Basic Search

```bash
# List all files in current directory
./find_my_files .

# Recursively list all files
./find_my_files -r .
```

### Pattern Matching

```bash
# Find all C++ source files
./find_my_files -r -n "*.cpp" src/

# Find files starting with "test"
./find_my_files -r -n "test*" .

# Case-insensitive search
./find_my_files -r -i -n "*readme*" .
```

### Regular Expressions

```bash
# Find test files matching pattern
./find_my_files -r -x -n "test_.*\.cpp" tests/

# Multiple extensions with regex
./find_my_files -r -x -n ".*\.(cpp|h|hpp)" .
```

### Content Search

```bash
# Find files containing "TODO"
./find_my_files -r -c "TODO" src/

# Search for function definitions (regex)
./find_my_files -r -x -c "void\s+\w+\(" src/

# Case-insensitive content search
./find_my_files -r -i -c "copyright" .
```

### Size Filters

```bash
# Find files larger than 1MB
./find_my_files -r --min-size 1048576 .

# Find files between 1KB and 100KB
./find_my_files -r --min-size 1024 --max-size 102400 .

# Find empty files
./find_my_files -r --max-size 0 .
```

### Parallel Scanning

```bash
# Use 4 threads for faster scanning
./find_my_files -r -j 4 /large/directory

# Use all available CPU cores
./find_my_files -r -j 0 .
```

### Ignore Patterns

```bash
# Use .gitignore to exclude files
./find_my_files -r --ignore .gitignore .

# Create custom ignore file
echo "*.o" > .myignore
echo "build/" >> .myignore
./find_my_files -r --ignore .myignore .
```

### Output Formats

```bash
# Detailed output with full metadata
./find_my_files -r --format detailed src/

# JSON output for parsing
./find_my_files -r --format json . > results.json

# Colored output for terminal
./find_my_files -r --color .
```

### Logging

```bash
# Info level logging
./find_my_files -r -v src/

# Debug level logging
./find_my_files -r -v -v src/

# Log to file
./find_my_files -r -v --log-file search.log .
```

### Complex Searches

```bash
# Find large C++ files with "TODO" comments
./find_my_files -r -e .cpp --min-size 10000 -c "TODO" src/

# Find recent test files (within last 7 days)
./find_my_files -r -n "*test*.cpp" --modified-after "7 days ago" .

# Exclude build artifacts and find headers
./find_my_files -r -e .h --ignore .gitignore --format json . > headers.json
```

## Building from Source

### Prerequisites

- CMake 3.14+
- C++17 compiler
- Google Test (automatically downloaded during build)

### Build Instructions

```bash
# Clone repository
git clone https://github.com/leeh8911/find_my_file.git
cd find_my_file

# Create build directory
mkdir -p build
cd build

# Configure and build
cmake ..
make

# Build executable will be in build/find_my_files
```

### Build Options

```bash
# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..
make

# Debug build (with debug symbols)
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Running Tests

### Unit Tests

```bash
cd build
./tests

# Run specific test suite
./tests --gtest_filter="FileScannerTest.*"
```

### Integration Tests

```bash
# Run all integration tests
cd test/integrationtest
./run_all.sh

# Run specific test
./uc_content_search.sh
```

## Architecture

Find My Files follows SOLID principles and uses modern C++ design patterns:

### Core Components

- **FileInfo**: Encapsulates file metadata
- **SearchCriteria**: Builder pattern for search filters
- **FileScanner**: Core scanning logic with parallel support
- **PatternMatcher**: Wildcard and regex matching
- **ContentSearcher**: File content searching
- **IgnorePatterns**: `.gitignore` style pattern support
- **OutputFormatter**: Strategy pattern for output formats
- **ThreadPool**: Parallel task execution
- **Logger**: Singleton logging system
- **CommandLineParser**: CLI argument parsing (SRP)

### Design Patterns

- **Singleton**: Logger for global logging access
- **Strategy**: OutputFormatter for different output formats
- **Builder**: SearchCriteria for complex search configurations
- **Factory**: Pattern creation and matching

For detailed architecture documentation, see [docs/architecture.md](docs/architecture.md).

## Performance

- **Sequential Scan**: ~10,000 files/second (depends on file system)
- **Parallel Scan (4 threads)**: ~30,000 files/second (3x improvement)
- **Content Search**: ~1,000 files/second (text files only)

Performance varies based on:
- File system type (SSD vs HDD)
- Number of CPU cores
- File sizes and types
- Search complexity

## Testing

- **81 Unit Tests**: Covering all core components
- **32 Integration Tests**: Real-world usage scenarios
- **Test Coverage**: >90% for critical paths

## Contributing

Contributions are welcome! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Development Workflow (Important!)

This project follows a **Phase-based branching model** with strict workflow rules.

**📋 Before You Start:**
1. Read [`.github/instructions/README.instructions.md`](.github/instructions/README.instructions.md) - Overview of all development rules
2. Read [`docs/todos.md`](docs/todos.md) - Current work items and phases
3. Read [`docs/architecture.md`](docs/architecture.md) - System design and structure

**🔀 Branching Strategy:**
```bash
# 1. Find next phase from todos.md
cat docs/todos.md | grep "Phase"

# 2. Create or switch to phase branch
git checkout -b phase/<phase-name>  # or git checkout phase/existing-phase

# 3. Work on phase branch (NEVER on main for code)
# - Write tests (test: commits)
# - Implement features (feat: commits)
# - Refactor (refactor: commits)
# - Auto-commit and push after each step

# 4. Merge when phase is complete
git checkout main
git merge phase/<phase-name>
git push origin main
```

**⚠️ Critical Rules:**
- ❌ **NEVER write code on `main` branch** (docs only)
- ✅ Always work on `phase/<name>` branches
- ✅ Follow TDD cycle: RED → GREEN → REFACTOR
- ✅ Auto-commit after each meaningful unit
- ✅ Auto-push immediately after commit

**📚 Key Documentation:**
- [`.github/instructions/workflow.instructions.md`](.github/instructions/workflow.instructions.md) - Complete workflow guide
- [`.github/instructions/tdd-cycle.instructions.md`](.github/instructions/tdd-cycle.instructions.md) - TDD guidelines
- [`.github/instructions/commit-style.instructions.md`](.github/instructions/commit-style.instructions.md) - Commit message format

### Development Setup

```bash
# Install development tools
sudo apt-get install clang-format clang-tidy

# Format code
clang-format -i src/*.cpp include/*.h

# Run linter
clang-tidy src/*.cpp -- -Iinclude
```

## Roadmap

### Completed

- ✅ Phase 1: Basic file scanning
- ✅ Phase 2: Search filtering
- ✅ Phase 3: Advanced search (regex, content search, ignore patterns)
- ✅ Phase 4: Parallel processing with ThreadPool
- ✅ Phase 5: Multiple output formats
- ✅ Phase 6: Error handling and logging

### Upcoming

- [ ] Phase 7: CI/CD pipeline with GitHub Actions
- [ ] Phase 8: Package manager support (apt, brew)
- [ ] Phase 9: Configuration file support (.findmyfilesrc)

### Future

- [ ] GUI version with Qt
- [ ] Database indexing for faster searches
- [ ] Network drive support
- [ ] Plugin system for custom filters

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Google Test for testing framework
- C++ Standard Library for filesystem support
- Contributors and testers

## Support

- **Documentation**: [docs/](docs/)
- **Issues**: [GitHub Issues](https://github.com/leeh8911/find_my_file/issues)
- **Discussions**: [GitHub Discussions](https://github.com/leeh8911/find_my_file/discussions)

---

**Made with ❤️ by leeh8911**
