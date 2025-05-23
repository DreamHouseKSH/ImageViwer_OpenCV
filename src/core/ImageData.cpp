#include "airphoto_viewer/ImageData.hpp"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <filesystem>
#include <exiv2/exiv2.hpp>
#include <exiv2/image.hpp>
#include <tiffio.h>

// TIFF 태그 정의 (표준 태그 중 일부)
#define TIFFTAG_IMAGEDESCRIPTION   270
#define TIFFTAG_MAKE               271
#define TIFFTAG_MODEL              272
#define TIFFTAG_ORIENTATION        274
#define TIFFTAG_XRESOLUTION        282
#define TIFFTAG_YRESOLUTION        283
#define TIFFTAG_RESOLUTIONUNIT     296
#define TIFFTAG_SOFTWARE           305
#define TIFFTAG_DATETIME           306
#define TIFFTAG_ARTIST             315
#define TIFFTAG_COLORMATRIX        33920
#define TIFFTAG_GDAL_METADATA      42112
#define TIFFTAG_GDAL_NODATA        42113

namespace fs = std::filesystem;

namespace airphoto {

ImageData::ImageData(const std::string& filepath) 
    : filepath_(filepath), 
      imageData_(std::make_shared<cv::Mat>()),
      metadata_(std::make_shared<ImageMetadata>()),
      currentPage_(0),
      pageCount_(1) {
    
    try {
        std::string ext = fs::path(filepath).extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        
        if (ext == ".tif" || ext == ".tiff") {
            // TIFF 파일인 경우 멀티페이지 처리
            TIFF* tif = TIFFOpen(filepath.c_str(), "r");
            if (tif) {
                // 페이지 수 계산
                do {
                    pageCount_++;
                } while (TIFFReadDirectory(tif));
                TIFFClose(tif);
                
                // 첫 번째 페이지 로드
                loadPage(0);
            } else {
                throw std::runtime_error("Failed to open TIFF file: " + filepath);
            }
        } else {
            // 일반 이미지 파일 로드
            *imageData_ = cv::imread(filepath, cv::IMREAD_UNCHANGED);
            if (imageData_->empty()) {
                throw std::runtime_error("Failed to load image: " + filepath);
            }
            extractMetadata();
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Error in ImageData: " + std::string(e.what()));
    }
}

bool ImageData::isLoaded() const {
    return !imageData_->empty();
}

std::shared_ptr<cv::Mat> ImageData::getImageData() const {
    return imageData_;
}

std::shared_ptr<ImageMetadata> ImageData::getMetadata() const {
    return metadata_;
}

bool ImageData::save(const std::string& filepath) const {
    if (imageData_->empty()) {
        return false;
    }
    
    std::vector<int> params;
    std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
    
    // 이미지 포맷에 따른 파라미터 설정
    if (ext == "jpg" || ext == "jpeg") {
        params = {cv::IMWRITE_JPEG_QUALITY, 95};
    } else if (ext == "png") {
        params = {cv::IMWRITE_PNG_COMPRESSION, 3};
    } else if (ext == "tiff" || ext == "tif") {
        params = {cv::IMWRITE_TIFF_COMPRESSION, 5};  // LZW 압축
    }
    
    try {
        return cv::imwrite(filepath, *imageData_, params);
    } catch (const cv::Exception& e) {
        std::cout << "Error saving image: " << e.what() << std::endl;
        return false;
    }
}

void ImageData::extractMetadata() {
    try {
        // Exiv2를 사용하여 메타데이터 추출
        auto image = Exiv2::ImageFactory::open(filepath_);
        if (!image.get()) {
            throw std::runtime_error("Could not open image file: " + filepath_);
        }
        
        image->readMetadata();
        
        // 기본 이미지 정보 설정
        metadata_->width = imageData_->cols;
        metadata_->height = imageData_->rows;
        metadata_->channels = imageData_->channels();
        metadata_->format = filepath_.substr(filepath_.find_last_of(".") + 1);
        
        // 색상 공간 설정
        if (imageData_->channels() == 1) {
            metadata_->colorSpace = "Grayscale";
        } else if (imageData_->channels() == 3) {
            metadata_->colorSpace = "BGR";  // OpenCV 기본 포맷
        } else if (imageData_->channels() == 4) {
            metadata_->colorSpace = "BGRA";  // OpenCV 기본 포맷
            metadata_->hasAlpha = true;
        }
        
        // EXIF 데이터 추출
        if (image->exifData().empty()) {
            return;
        }
        
        // EXIF 데이터를 맵에 저장
        for (const auto& md : image->exifData()) {
            std::string key = md.key();
            std::string value = md.toString();
            metadata_->exifData[key] = value;
            
            // 특정 EXIF 태그 처리
            if (key == "Exif.Image.Make") {
                metadata_->cameraMake = value;
            } else if (key == "Exif.Image.Model") {
                metadata_->cameraModel = value;
            } else if (key == "Exif.Photo.ExposureTime") {
                try {
                    metadata_->exposureTime = std::stod(value);
                } catch (...) {}
            } else if (key == "Exif.Photo.FNumber") {
                try {
                    metadata_->fNumber = std::stod(value);
                } catch (...) {}
            } else if (key == "Exif.Photo.ISOSpeedRatings") {
                try {
                    metadata_->isoSpeed = std::stoi(value);
                } catch (...) {}
            } else if (key == "Exif.Photo.FocalLength") {
                try {
                    metadata_->focalLength = std::stod(value);
                } catch (...) {}
            } else if (key == "Exif.Image.DateTime" || key == "Exif.Photo.DateTimeOriginal") {
                metadata_->datetimeOriginal = value;
            } else if (key.find("GPS") == 0) {
                // GPS 태그 처리 (간소화된 버전)
                if (key == "GPS.GPSLatitude") {
                    std::string ref = getExifValue(metadata_->exifData, "GPS.GPSLatitudeRef");
                    metadata_->gpsLatitude = convertGpsCoordinate(value, ref);
                } else if (key == "GPS.GPSLongitude") {
                    std::string ref = getExifValue(metadata_->exifData, "GPS.GPSLongitudeRef");
                    metadata_->gpsLongitude = convertGpsCoordinate(value, ref);
                } else if (key == "GPS.GPSAltitude") {
                    try {
                        std::string ref = getExifValue(metadata_->exifData, "GPS.GPSAltitudeRef");
                        double alt = std::stod(value);
                        if (ref == "1") {  // 해수면 아래
                            alt = -alt;
                        }
                        metadata_->gpsAltitude = alt;
                    } catch (...) {}
                }
            }
        }
        
        // 색상 프로파일 추출 (간소화된 버전)
        if (image->iccProfileDefined()) {
            auto profile = std::make_shared<ImageMetadata::ColorProfile>();
            profile->profileType = "ICC";
            // Exiv2 0.28.0 이상에서는 iccProfile()이 DataBuf를 반환
            auto iccProfile = image->iccProfile();
            profile->profileSize = iccProfile.size();
            // 실제 프로파일 데이터는 크기가 클 수 있으므로 여기서는 크기만 저장
            metadata_->colorProfile = profile;
        }
        
    } catch (const std::exception& e) {
        std::cout << "Error extracting metadata: " << e.what() << std::endl;
    }
}

std::string ImageData::getExifValue(const std::map<std::string, std::string>& exifData, const std::string& tag) const {
    auto it = exifData.find(tag);
    if (it != exifData.end()) {
        return it->second;
    }
    return "";
}

double ImageData::convertGpsCoordinate(const std::string& coord, const std::string& ref) const {
    try {
        // 도/분/초 형식의 좌표를 파싱 (예: "40/1 26/1 0/1")
        std::istringstream iss(coord);
        std::string part;
        double degrees = 0.0, minutes = 0.0, seconds = 0.0;
        
        // 도 추출
        if (std::getline(iss, part, ' ')) {
            size_t pos = part.find('/');
            if (pos != std::string::npos) {
                double num = std::stod(part.substr(0, pos));
                double den = std::stod(part.substr(pos + 1));
                degrees = num / den;
            }
        }
        
        // 분 추출
        if (std::getline(iss, part, ' ')) {
            size_t pos = part.find('/');
            if (pos != std::string::npos) {
                double num = std::stod(part.substr(0, pos));
                double den = std::stod(part.substr(pos + 1));
                minutes = num / den;
            }
        }
        
        // 초 추출
        if (std::getline(iss, part, ' ')) {
            size_t pos = part.find('/');
            if (pos != std::string::npos) {
                double num = std::stod(part.substr(0, pos));
                double den = std::stod(part.substr(pos + 1));
                seconds = num / den;
            }
        }
        
        // 도 단위로 변환
        double decimal = degrees + (minutes / 60.0) + (seconds / 3600.0);
        
        // 방향에 따라 부호 결정
        if (ref == "S" || ref == "W") {
            decimal = -decimal;
        }
        
        return decimal;
        
    } catch (const std::exception& e) {
        std::cout << "Error converting GPS coordinate: " << e.what() << std::endl;
        return 0.0;
    }
}

// getPageCount is implemented inline in the header

bool ImageData::loadPage(int pageIndex) {
    if (pageIndex < 0 || pageIndex >= pageCount_) {
        std::cerr << "Invalid page index: " << pageIndex << std::endl;
        return false;
    }

    std::string ext = fs::path(filepath_).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    if (ext == ".tif" || ext == ".tiff") {
        TIFF* tif = nullptr;
        try {
            tif = TIFFOpen(filepath_.c_str(), "r");
            if (!tif) {
                std::cerr << "Failed to open TIFF file: " << filepath_ << std::endl;
                return false;
            }

            // 지정된 페이지로 이동
            for (int i = 0; i <= pageIndex; i++) {
                if (i > 0 && !TIFFReadDirectory(tif)) {
                    std::cerr << "Failed to read directory for page " << pageIndex << std::endl;
                    TIFFClose(tif);
                    return false;
                }
            }

            // 이미지 크기 가져오기
            uint32_t width = 0, height = 0;
            TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &width);
            TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &height);

            if (width == 0 || height == 0) {
                std::cerr << "Invalid image dimensions: " << width << "x" << height << std::endl;
                TIFFClose(tif);
                return false;
            }


            // 메모리 할당 (4채널로 읽은 후 필요한 변환 수행)
            cv::Mat rgbaImage(height, width, CV_8UC4);
            
            // TIFF에서 이미지 읽기 (RGBA 형식으로 읽음)
            if (TIFFReadRGBAImage(tif, width, height, reinterpret_cast<uint32_t*>(rgbaImage.data), 0)) {
                // OpenCV는 BGR 순서를 사용하므로 변환
                cv::Mat bgrImage;
                cv::cvtColor(rgbaImage, bgrImage, cv::COLOR_RGBA2BGR);
                *imageData_ = bgrImage;
                currentPage_ = pageIndex;
                
                // 메타데이터 추출
                extractMetadata();

                TIFFClose(tif);
                return true;
            } else {
                std::cerr << "Failed to read TIFF image data for page " << pageIndex << std::endl;
                TIFFClose(tif);
                return false;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error loading TIFF page: " << e.what() << std::endl;
            if (tif) {
                TIFFClose(tif);
            }
            return false;
        }
    } else if (pageIndex == 0) {
        // 일반 이미지의 경우 첫 페이지만 지원
        *imageData_ = cv::imread(filepath_, cv::IMREAD_UNCHANGED);
        currentPage_ = 0;
        extractMetadata();
        return true;
    }

    return false;
}
} // namespace airphoto
