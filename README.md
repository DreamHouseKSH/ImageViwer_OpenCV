# 항공사진 뷰어 (Aerial Photo Viewer)

C++와 Qt6, OpenCV를 사용한 고성능 항공사진 뷰어 애플리케이션입니다. 대용량 항공사진을 빠르게 로드하고, 다양한 측정 도구를 활용하여 정밀한 분석이 가능합니다.

## ✨ 주요 기능

- **대용량 이미지 지원**
  - TIFF, JPEG2000 등 다양한 형식 지원
  - 멀티스레딩을 통한 빠른 이미지 로딩
  - 효율적인 메모리 관리를 통한 대용량 파일 처리

- **뷰어 기능**
  - 부드러운 확대/축소 및 패닝
  - 이미지 회전 및 뒤집기
  - 다양한 보기 모드 (원본, 창에 맞춤, 실제 크기)

- **측정 도구**
  - 거리 측정: 두 지점 간의 거리 측정
  - 면적 측정: 다각형 영역의 면적 계산
  - 실시간 미리보기 및 측정값 표시
  - 사용자 정의 가능한 측정 스타일

- **고급 기능**
  - 멀티페이지 TIFF 지원
  - 이미지 메타데이터 추출
  - 사용자 정의 가능한 단축키

## 📦 빌드 요구사항

- **필수 패키지**
  - C++17 호환 컴파일러 (GCC 8+, Clang 7+, MSVC 2019+)
  - CMake 3.16 이상
  - Qt6 (Core, Gui, Widgets, Qml, Quick, QuickControls2 모듈)
  - OpenCV 4.5.0 이상
  - exiv2 (이미지 메타데이터 처리)
  - libtiff (TIFF 파일 지원)

- **추천 개발 환경**
  - macOS: Xcode 12+
  - Linux: GCC 9+ / Clang 10+
  - Windows: Visual Studio 2019+

## 🛠️ 빌드 및 실행

### 기본 빌드 (권장)

```bash
# 저장소 클론 및 이동
git clone https://github.com/yourusername/airphoto-viewer.git
cd airphoto-viewer

# 빌드 스크립트 실행 (릴리스 모드)
chmod +x build.sh
./build.sh

# 실행
./airphoto_viewer [이미지 파일 경로]
```

### 고급 빌드 옵션

```bash
# 디버그 모드로 빌드
./build.sh --debug

# 빌드 디렉토리 정리 후 재빌드
./build.sh --clean

# 빌드 후 측정 도구 예제 실행
./build.sh --run-example

# 병렬 빌드 (예: 8개 작업)
./build.sh -j8
```

### CMake를 직접 사용하는 경우

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -- -j$(nproc)
```

## 🖥️ 사용 방법

### 기본 조작

| 기능 | 단축키 |
|------|--------|
| 이미지 열기 | `Ctrl+O` |
| 이미지 저장 | `Ctrl+S` |
| 확대 | `Ctrl++` 또는 `Ctrl+마우스 휠 위로` |
| 축소 | `Ctrl+-` 또는 `Ctrl+마우스 휠 아래로` |
| 원본 크기 | `Ctrl+0` |
| 창에 맞추기 | `Ctrl+F` |
| 회전 (시계 방향) | `Ctrl+R` |
| 회전 (반시계 방향) | `Ctrl+Shift+R` |
| 측정 모드 전환 | `M` |
| 측정 지우기 | `Delete` 또는 `Esc` |

### 측정 도구 사용법

1. **거리 측정**
   - 측정 모드로 전환 (`M` 키 또는 툴바 버튼)
   - 측정 시작점 클릭
   - 측정 종료점 클릭
   - 측정 완료 후 `Enter` 또는 더블 클릭

2. **면적 측정**
   - 측정 모드에서 면적 측정으로 전환
   - 다각형의 각 꼭지점을 순서대로 클릭
   - 마지막 점을 첫 번째 점에 연결하려면 더블 클릭 또는 `Enter`

3. **측정 조작**
   - `Delete` 키로 마지막 점 삭제
   - `Esc` 키로 현재 측정 취소
   - 측정값은 실시간으로 표시되며, 픽셀당 실제 거리(pixelScale)에 따라 계산됩니다.

### 고급 기능

- **이미지 메타데이터 보기**: `Ctrl+I`
- **측정 단위 변경**: 설정 메뉴에서 픽셀당 거리 조정
- **측정 스타일 사용자 정의**: 설정에서 선 색상, 두께, 텍스트 스타일 변경

## 📝 문서

- [측정 도구 API 문서](./docs/measurement_tool.md)
- [개발자 가이드](./docs/DEVELOPMENT.md)
- [기여 가이드라인](./docs/CONTRIBUTING.md)

## 🤝 기여하기

버그 리포트, 기능 요청, 풀 리퀘스트는 언제나 환영합니다! 
기여하기 전에 [기여 가이드라인](./docs/CONTRIBUTING.md)을 확인해주세요.

## 📄 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 [LICENSE](./LICENSE) 파일을 참조하세요.

---

<div align="center">
  <p>만든이: Your Name</p>
  <p>© 2025 AirPhoto Viewer Project</p>
</div>
