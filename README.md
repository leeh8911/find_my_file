# Find My Files

파일 검색 및 탐색을 위한 C++ 명령줄 도구

## 프로젝트 개요

Find My Files는 디렉토리 내의 파일을 효율적으로 검색하고 탐색하기 위한 C++17 기반 명령줄 도구입니다. 빠른 파일 시스템 탐색, 다양한 필터링 옵션, 그리고 직관적인 사용자 인터페이스를 제공합니다.

## 기능

### Phase 1-2 완료 ✅
- ✅ 기본 파일 시스템 탐색
- ✅ 재귀적 디렉토리 스캔
- ✅ 최대 깊이 제한 옵션
- ✅ 심볼릭 링크 처리
- ✅ 파일 정보 표시 (이름, 크기, 타입)
- ✅ 파일 이름 검색 (와일드카드 지원: `*`, `?`)
- ✅ 대소문자 구분/비구분 검색
- ✅ 확장자 필터링 (단일/다중)
- ✅ 경로 패턴 검색
- ✅ 파일 크기 필터링 (최소/최대)
- ✅ 파일 타입 필터 (파일만/디렉토리만)

### 개발 예정
- 정규식(Regex) 검색
- 파일 내용 검색
- 성능 최적화 (멀티스레딩, 캐싱)
- 다양한 출력 포맷 (JSON, 상세 정보)

## 빌드 방법

### 요구사항
- C++17 호환 컴파일러 (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.14 이상
- Git

### 빌드 단계

```bash
# 저장소 클론
git clone https://github.com/leeh8911/find_my_file.git
cd find_my_files

# 빌드 디렉토리 생성 및 빌드
mkdir build
cd build
cmake ..
make

# 테스트 실행
./tests

# 프로그램 실행
./find_my_files --help
```

## 사용 방법

### 기본 사용법

```bash
# 현재 디렉토리 스캔
./find_my_files .

# 특정 디렉토리 스캔
./find_my_files /path/to/directory

# 재귀적으로 스캔
./find_my_files -r /path/to/directory

# 최대 깊이 제한
./find_my_files -r -d 2 /path/to/directory

# 심볼릭 링크 따라가기
./find_my_files -r -l /path/to/directory
```

### 검색 옵션

```bash
# 이름으로 검색 (와일드카드 지원)
./find_my_files -r -n '*.cpp' .

# 대소문자 구분 없이 검색
./find_my_files -r -i -n '*test*' .

# 확장자로 필터링
./find_my_files -r -e .h .
./find_my_files -r -e .h -e .cpp .  # 여러 확장자

# 경로 패턴 검색
./find_my_files -r -p '*/tests/*' .

# 파일만 표시
./find_my_files -r -f .

# 디렉토리만 표시
./find_my_files -r -D .

# 파일 크기로 필터링
./find_my_files -r --min-size 1000 --max-size 100000 .
```

### 옵션

**검색 옵션:**
- `-n, --name PATTERN`: 파일 이름으로 검색 (와일드카드 `*`, `?` 지원)
- `-e, --ext EXT`: 확장자로 필터링 (예: .cpp, .h)
- `-p, --path PATTERN`: 경로 패턴으로 검색
- `-i, --ignore-case`: 대소문자 구분 없이 검색

**파일 타입 옵션:**
- `-f, --files-only`: 파일만 표시 (디렉토리 제외)
- `-D, --dirs-only`: 디렉토리만 표시

**크기 옵션:**
- `--min-size SIZE`: 최소 파일 크기 (바이트)
- `--max-size SIZE`: 최대 파일 크기 (바이트)

**탐색 옵션:**
- `-r, --recursive`: 디렉토리를 재귀적으로 스캔
- `-d, --max-depth N`: 최대 재귀 깊이 설정
- `-l, --follow-links`: 심볼릭 링크 따라가기

**기타 옵션:**
- `-h, --help`: 도움말 표시

## 프로젝트 구조

```
find_my_files/
├── CMakeLists.txt         # CMake 빌드 설정
├── README.md              # 프로젝트 문서
├── docs/                  # 추가 문서
│   ├── architecture.md    # 아키텍처 설계 문서
│   └── todos.md          # 개발 계획 및 TODO
├── include/              # 헤더 파일
│   ├── file_info.h       # 파일 정보 클래스
│   ├── search_result.h   # 검색 결과 클래스
│   ├── file_scanner.h    # 파일 스캐너 클래스
│   ├── search_criteria.h # 검색 조건 클래스
│   └── pattern_matcher.h # 패턴 매칭 유틸리티
├── src/                  # 소스 파일
│   ├── file_info.cpp
│   ├── search_result.cpp
│   ├── file_scanner.cpp
│   ├── search_criteria.cpp
│   ├── pattern_matcher.cpp
│   └── main.cpp          # 메인 프로그램
└── tests/                # 테스트 파일
    ├── test_file_info.cpp
    ├── test_file_scanner.cpp
    ├── test_pattern_matcher.cpp
    └── test_search_criteria.cpp
```

## 개발 로드맵

프로젝트는 9개의 Phase로 나뉘어 개발됩니다:

1. ✅ **Phase 1**: 프로젝트 기본 구조 설정 (완료)
2. ✅ **Phase 2**: 기본 검색 기능 (완료)
3. **Phase 3**: 고급 검색 및 필터링 (진행 예정)
4. **Phase 4**: 성능 최적화
5. **Phase 5**: CLI 인터페이스 개선
6. **Phase 6**: 에러 처리 및 로깅
7. **Phase 7**: 테스트 확장
8. **Phase 8**: 문서화 및 배포
9. **Phase 9**: 추가 기능 (선택사항)

자세한 내용은 [docs/todos.md](docs/todos.md)를 참조하세요.

## 테스트

프로젝트는 Google Test 프레임워크를 사용합니다.

```bash
# 빌드 디렉토리에서 테스트 실행
cd build
./tests

# 또는 CTest 사용
ctest --output-on-failure
```

현재 테스트 커버리지: 29개 테스트, 모두 통과 ✅

## 기여 방법

1. 이 저장소를 Fork 합니다
2. 새로운 기능 브랜치를 생성합니다 (`git checkout -b feature/amazing-feature`)
3. 변경사항을 커밋합니다 (`git commit -m 'Add some amazing feature'`)
4. 브랜치에 Push 합니다 (`git push origin feature/amazing-feature`)
5. Pull Request를 생성합니다

## 코딩 스타일

- C++17 표준 준수
- Google C++ Style Guide 기반
- `.clang-format` 및 `.clang-tidy` 설정 사용

## 라이센스

이 프로젝트는 MIT 라이센스 하에 배포됩니다.

## 작성자

- leeh8911

## 문의

이슈나 질문이 있으시면 GitHub Issues를 통해 알려주세요.
