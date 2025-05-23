"""
이미지 데이터를 관리하는 핵심 모듈입니다.
이 모듈은 이미지 로딩, 메모리 관리, 기본 속성 관리를 담당합니다.
"""

import os
from dataclasses import dataclass, asdict
from datetime import datetime
from pathlib import Path
from typing import Any, Dict, List, Optional, Tuple, Union

import cv2
import numpy as np
from PIL import Image, ImageCms, ExifTags, TiffTags
from PIL.TiffImagePlugin import IFDRational


@dataclass
class ImageMetadata:
    """이미지 메타데이터를 저장하는 데이터 클래스입니다.
    
    속성:
        width (int): 이미지 너비 (픽셀 단위)
        height (int): 이미지 높이 (픽셀 단위)
        channels (int): 색상 채널 수 (예: RGB=3, RGBA=4)
        dpi (Tuple[float, float]): 이미지 해상도 (x, y)
        color_space (str): 색상 공간 (예: 'RGB', 'RGBA', 'L')
        format (str): 이미지 포맷 (예: 'JPEG', 'PNG', 'TIFF')
        has_alpha (bool): 알파 채널 존재 여부
        
        # EXIF 데이터
        exif_data (dict): EXIF 메타데이터 (사용 가능한 경우)
        
        # 지리참조 정보
        geo_reference (dict): 지리참조 정보 (사용 가능한 경우)
        
        # 색상 프로파일 정보
        color_profile (dict): 내장된 색상 프로파일 정보 (사용 가능한 경우)
        
        # 카메라 정보
        camera_make (str): 카메라 제조사
        camera_model (str): 카메라 모델
        
        # 촬영 정보
        datetime_original (str): 원본 촬영 일시
        exposure_time (float): 노출 시간 (초)
        f_number (float): 조리개 값
        iso_speed (int): ISO 감도
        focal_length (float): 초점 거리 (mm)
        
        # GPS 정보
        gps_latitude (float): 위도 (도 단위)
        gps_longitude (float): 경도 (도 단위)
        gps_altitude (float): 고도 (미터 단위)
    """
    # 기본 이미지 정보
    width: int = 0
    height: int = 0
    channels: int = 0
    dpi: Tuple[float, float] = (0.0, 0.0)
    color_space: str = ""
    format: str = ""
    has_alpha: bool = False
    
    # EXIF 데이터
    exif_data: dict = None
    
    # 지리참조 정보
    geo_reference: dict = None
    
    # 색상 프로파일 정보
    color_profile: dict = None
    
    # 카메라 정보
    camera_make: str = ""
    camera_model: str = ""
    
    # 촬영 정보
    datetime_original: str = ""
    exposure_time: float = 0.0
    f_number: float = 0.0
    iso_speed: int = 0
    focal_length: float = 0.0
    
    # GPS 정보
    gps_latitude: float = 0.0
    gps_longitude: float = 0.0
    gps_altitude: float = 0.0
    
    def to_dict(self) -> Dict[str, Any]:
        """메타데이터를 사전 형태로 변환합니다."""
        result = asdict(self)
        
        # datetime 객체를 문자열로 변환
        if 'datetime_original' in result and result['datetime_original']:
            if isinstance(result['datetime_original'], datetime):
                result['datetime_original'] = result['datetime_original'].isoformat()
        
        # EXIF 데이터가 bytes 타입인 경우 문자열로 변환
        if 'exif_data' in result and result['exif_data']:
            exif_data = {}
            for k, v in result['exif_data'].items():
                if isinstance(v, bytes):
                    try:
                        exif_data[k] = v.decode('utf-8', errors='replace')
                    except (UnicodeDecodeError, AttributeError):
                        exif_data[k] = str(v)
                else:
                    exif_data[k] = v
            result['exif_data'] = exif_data
        
        return result


class ImageData:
    """이미지 데이터를 관리하는 핵심 클래스입니다.
    
    이 클래스는 이미지 로딩, 메모리 관리, 기본 속성 관리를 담당합니다.
    
    속성:
        filepath (Path): 이미지 파일 경로
        _data (Optional[np.ndarray]): 이미지 데이터 (numpy 배열)
        _metadata (ImageMetadata): 이미지 메타데이터
    """
    
    def __init__(self, filepath: Optional[Union[str, Path]] = None):
        """ImageData 인스턴스를 초기화합니다.
        
        Args:
            filepath: 로드할 이미지 파일 경로. None인 경우 빈 인스턴스가 생성됩니다.
        """
        self.filepath = Path(filepath) if filepath else None
        self._data: Optional[np.ndarray] = None
        self._metadata = ImageMetadata()
        
        if filepath:
            self.load()
    
    def load(self, filepath: Optional[Union[str, Path]] = None) -> None:
        """이미지 파일을 로드합니다.
        
        Args:
            filepath: 로드할 이미지 파일 경로. None인 경우 기존 filepath 사용
            
        Raises:
            FileNotFoundError: 파일이 존재하지 않는 경우
            ValueError: 지원하지 않는 이미지 포맷인 경우
            IOError: 이미지 로딩에 실패한 경우
        """
        if filepath:
            self.filepath = Path(filepath)
            
        if not self.filepath or not self.filepath.exists():
            raise FileNotFoundError(f"이미지 파일을 찾을 수 없습니다: {self.filepath}")
        
        try:
            # OpenCV로 이미지 로드 (BGR 형식)
            self._data = cv2.imread(str(self.filepath), cv2.IMREAD_UNCHANGED)
            
            if self._data is None:
                raise IOError(f"이미지 로딩에 실패했습니다: {self.filepath}")
                
            # 메타데이터 추출
            self._extract_metadata()
            
        except Exception as e:
            self._data = None
            raise IOError(f"이미지 로딩 중 오류가 발생했습니다: {e}")
    
    def unload(self) -> None:
        """이미지 데이터를 메모리에서 해제합니다."""
        self._data = None
    
    def _convert_to_degrees(self, value: tuple) -> float:
        """EXIF의 도/분/초 형식을 도 단위로 변환합니다."""
        try:
            d = float(value[0])
            m = float(value[1])
            s = float(value[2])
            return d + (m / 60.0) + (s / 3600.0)
        except (TypeError, ValueError, IndexError):
            return 0.0

    def _extract_exif_data(self, img: Image.Image) -> None:
        """이미지에서 EXIF 데이터를 추출합니다."""
        try:
            if not hasattr(img, '_getexif') or not img._getexif():
                return
                
            exif_data = {}
            for tag, value in img._getexif().items():
                if tag in ExifTags.TAGS:
                    exif_data[ExifTags.TAGS[tag]] = value
            
            self._metadata.exif_data = exif_data
            
            # 카메라 정보 추출
            self._metadata.camera_make = exif_data.get('Make', '')
            self._metadata.camera_model = exif_data.get('Model', '')
            
            # 촬영 정보 추출
            self._metadata.datetime_original = exif_data.get('DateTimeOriginal', '')
            
            # 노출 시간 (초 단위로 변환)
            exposure_time = exif_data.get('ExposureTime')
            if isinstance(exposure_time, tuple) and len(exposure_time) == 2:
                try:
                    self._metadata.exposure_time = float(exposure_time[0]) / float(exposure_time[1])
                except (TypeError, ValueError, ZeroDivisionError):
                    pass
            
            # 조리개 값
            fnumber = exif_data.get('FNumber')
            if isinstance(fnumber, tuple) and len(fnumber) == 2:
                try:
                    self._metadata.f_number = float(fnumber[0]) / float(fnumber[1])
                except (TypeError, ValueError, ZeroDivisionError):
                    pass
            
            # ISO 감도
            try:
                self._metadata.iso_speed = int(exif_data.get('ISOSpeedRatings', 0))
            except (TypeError, ValueError):
                pass
            
            # 초점 거리 (mm)
            focal_length = exif_data.get('FocalLength')
            if isinstance(focal_length, tuple) and len(focal_length) == 2:
                try:
                    self._metadata.focal_length = float(focal_length[0]) / float(focal_length[1])
                except (TypeError, ValueError, ZeroDivisionError):
                    pass
            
            # GPS 정보 추출
            gps_info = exif_data.get('GPSInfo')
            if gps_info and isinstance(gps_info, dict):
                self._extract_gps_info(gps_info)
                
        except Exception as e:
            print(f"EXIF 데이터 추출 중 오류 발생: {e}")
    
    def _extract_gps_info(self, gps_info: dict) -> None:
        """GPS 정보를 추출합니다."""
        try:
            # 위도 추출
            gps_latitude = gps_info.get(2)
            gps_latitude_ref = gps_info.get(1, 'N')
            
            if gps_latitude and len(gps_latitude) == 3:
                lat = self._convert_to_degrees(gps_latitude)
                if gps_latitude_ref == 'S':
                    lat = -lat
                self._metadata.gps_latitude = lat
            
            # 경도 추출
            gps_longitude = gps_info.get(4)
            gps_longitude_ref = gps_info.get(3, 'E')
            
            if gps_longitude and len(gps_longitude) == 3:
                lon = self._convert_to_degrees(gps_longitude)
                if gps_longitude_ref == 'W':
                    lon = -lon
                self._metadata.gps_longitude = lon
            
            # 고도 추출
            gps_altitude = gps_info.get(6)
            gps_altitude_ref = gps_info.get(5, 0)
            
            if gps_altitude and len(gps_altitude) == 1:
                try:
                    alt = float(gps_altitude[0])
                    if gps_altitude_ref == 1:  # 해수면 아래
                        alt = -alt
                    self._metadata.gps_altitude = alt
                except (TypeError, ValueError):
                    pass
                    
        except Exception as e:
            print(f"GPS 정보 추출 중 오류 발생: {e}")
    
    def _extract_color_profile(self, img: Image.Image) -> None:
        """색상 프로파일 정보를 추출합니다."""
        try:
            if 'icc_profile' in img.info:
                icc_profile = img.info['icc_profile']
                if isinstance(icc_profile, bytes):
                    self._metadata.color_profile = {
                        'profile_size': len(icc_profile),
                        'profile_type': 'ICC'
                    }
                    
                    # 색상 공간에 따라 color_space 업데이트
                    if 'Adobe RGB' in str(icc_profile):
                        self._metadata.color_space = 'Adobe RGB'
                    elif 'sRGB' in str(icc_profile):
                        self._metadata.color_space = 'sRGB'
                    elif 'ProPhoto RGB' in str(icc_profile):
                        self._metadata.color_space = 'ProPhoto RGB'
                    
        except Exception as e:
            print(f"색상 프로파일 추출 중 오류 발생: {e}")
    
    def _extract_metadata(self) -> None:
        """이미지로부터 메타데이터를 추출합니다."""
        if self._data is None or self.filepath is None:
            return
        
        try:
            # 기본 속성 설정
            height, width = self._data.shape[:2]
            channels = 1 if len(self._data.shape) == 2 else self._data.shape[2]
            
            # 색상 공간 결정
            if channels == 1:
                color_space = 'L'  # 그레이스케일
            elif channels == 3:
                color_space = 'RGB'
            elif channels == 4:
                color_space = 'RGBA'
            else:
                color_space = 'UNKNOWN'
            
            self._metadata = ImageMetadata(
                width=width,
                height=height,
                channels=channels,
                has_alpha=channels in [2, 4],
                color_space=color_space,
                format=os.path.splitext(str(self.filepath))[1].upper().lstrip('.')
            )
            
            # PIL을 사용하여 추가 메타데이터 추출
            with Image.open(self.filepath) as img:
                # DPI 정보 추출
                dpi = (0, 0)
                if 'dpi' in img.info:
                    dpi = img.info['dpi']
                    if isinstance(dpi, tuple) and len(dpi) == 2:
                        self._metadata.dpi = (float(dpi[0]), float(dpi[1]))
                
                # EXIF 데이터 추출
                self._extract_exif_data(img)
                
                # 색상 프로파일 추출
                self._extract_color_profile(img)
                
                # 지리참조 정보 초기화 (나중에 구현 예정)
                self._metadata.geo_reference = {}
                
        except Exception as e:
            print(f"메타데이터 추출 중 오류 발생: {e}")
    
    @property
    def data(self) -> Optional[np.ndarray]:
        """이미지 데이터를 반환합니다."""
        return self._data
    
    @property
    def metadata(self) -> ImageMetadata:
        """이미지 메타데이터를 반환합니다."""
        return self._metadata
    
    @property
    def is_loaded(self) -> bool:
        """이미지가 로드되었는지 여부를 반환합니다."""
        return self._data is not None
    
    def __del__(self):
        """객체 소멸 시 리소스를 정리합니다."""
        self.unload()


def load_image(filepath: Union[str, Path]) -> ImageData:
    """이미지 파일을 로드하여 ImageData 인스턴스를 반환합니다.
    
    Args:
        filepath: 로드할 이미지 파일 경로
        
    Returns:
        ImageData: 로드된 이미지 데이터
        
    Raises:
        FileNotFoundError: 파일이 존재하지 않는 경우
        ValueError: 지원하지 않는 이미지 포맷인 경우
        IOError: 이미지 로딩에 실패한 경우
    """
    return ImageData(filepath)
