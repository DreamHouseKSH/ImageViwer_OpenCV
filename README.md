# 항공사진 뷰어 (Aerial Photo Viewer)

C++와 Qt6, OpenCV를 사용한 고성능 항공사진 뷰어 애플리케이션입니다.

## 기능

- 대용량 TIFF 이미지 지원
- 빠른 이미지 로딩 및 렌더링
- 확대/축소, 이동, 회전 등의 기본 뷰어 기능
- 멀티스레딩을 통한 원활한 대용량 이미지 처리

## 빌드 요구사항

- C++17 호환 컴파일러 (GCC 8+, Clang 7+, MSVC 2019+)
- CMake 3.10 이상
- Qt6 (Core, Gui, Widgets 모듈)
- OpenCV 4.5.0 이상

## 빌드 방법

1. 저장소 클론:
   ```bash
   git clone https://github.com/yourusername/airphoto-viewer.git
   cd airphoto-viewer
   ```

2. 빌드 디렉토리 생성 및 이동:
   ```bash
   mkdir build
   cd build
   ```

3. CMake로 프로젝트 구성:
   ```bash
   cmake ..
   ```

4. 빌드:
   ```bash
   cmake --build .
   ```

5. 실행:
   ```bash
   ./airphoto_viewer [이미지 파일 경로]
   ```

## 사용 방법

1. **이미지 열기**: '파일 > 열기' 메뉴 또는 `Ctrl+O` 단축키 사용
2. **확대/축소**: 
   - `Ctrl++` 또는 `Ctrl+마우스 휠 위로`: 확대
   - `Ctrl+-` 또는 `Ctrl+마우스 휠 아래로`: 축소
   - `Ctrl+0`: 원본 크기로 보기
3. **이동**: 스크롤 바 또는 마우스 드래그로 이미지 이동
4. **창에 맞추기**: `Ctrl+F` 또는 '보기 > 창에 맞추기' 메뉴

## 라이선스

이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 LICENSE 파일을 참조하세요.
