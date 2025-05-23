#include "airphoto_viewer/ImageData.hpp"
#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>
#include <sstream>

int main() {
    try {
        // 테스트할 TIFF 파일 경로
        std::string tiffPath = "/Users/dhkim/Devs/ImageViwer_OpenCV/test_images/36701013.tif";
        
        // 이미지 로드
        auto image = std::make_unique<airphoto::ImageData>(tiffPath);
        
        if (!image->isLoaded()) {
            std::cerr << "이미지 로드 실패: " << tiffPath << std::endl;
            return 1;
        }
        
        // 기본 정보 출력
        std::cout << "=== 이미지 정보 ===" << std::endl;
        std::cout << "파일 경로: " << tiffPath << std::endl;
        std::cout << "이미지 크기: " << image->getImageData()->cols << " x " 
                  << image->getImageData()->rows << std::endl;
        
        // 페이지 정보 출력
        int pageCount = image->getPageCount();
        std::cout << "총 페이지 수: " << pageCount << std::endl;
        
        // 각 페이지별로 정보 출력
        for (int i = 0; i < pageCount; i++) {
            if (image->loadPage(i)) {
                auto img = image->getImageData();
                auto meta = image->getMetadata();
                
                std::cout << "\n--- 페이지 " << (i+1) << " ---" << std::endl;
                std::cout << "  크기: " << img->cols << " x " << img->rows << std::endl;
                std::cout << "  채널 수: " << img->channels() << std::endl;
                std::cout << "  색상 공간: " << meta->colorSpace << std::endl;
                
                // 메타데이터 출력
                std::cout << "  포맷: " << meta->format << std::endl;
                std::cout << "  해상도: " << meta->dpi.first << " x " << meta->dpi.second << " DPI" << std::endl;
                
                // 카메라 정보 출력
                if (!meta->cameraMake.empty() || !meta->cameraModel.empty()) {
                    std::cout << "  카메라: " << meta->cameraMake << " " << meta->cameraModel << std::endl;
                }
                
                // 촬영 일시 출력
                if (!meta->datetimeOriginal.empty()) {
                    std::cout << "  촬영일시: " << meta->datetimeOriginal << std::endl;
                }
                
                // GPS 정보 출력
                if (meta->gpsLatitude != 0.0 || meta->gpsLongitude != 0.0) {
                    std::cout << "  위치: " << std::fixed << std::setprecision(6) 
                              << meta->gpsLatitude << "°, " 
                              << meta->gpsLongitude << "°"
                              << " (고도: " << meta->gpsAltitude << "m)" << std::endl;
                }
                
                // 노출 정보 출력
                if (meta->exposureTime > 0) {
                    if (meta->exposureTime < 1.0) {
                        std::cout << "  노출: 1/" << (1.0 / meta->exposureTime) << "초";
                    } else {
                        std::cout << "  노출: " << meta->exposureTime << "초";
                    }
                    std::cout << ", 조리개: f/" << meta->fNumber;
                    std::cout << ", ISO: " << meta->isoSpeed << std::endl;
                }
            } else {
                std::cerr << "페이지 " << (i+1) << " 로드 실패" << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "오류 발생: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
