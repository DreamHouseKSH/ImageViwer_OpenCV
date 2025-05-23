"""
이미지 데이터를 관리하는 핵심 모듈입니다.
이 모듈은 이미지 로딩, 메모리 관리, 기본 속성 관리를 담당합니다.
"""

import os
from dataclasses import dataclass
from pathlib import Path
from typing import Optional, Tuple, Union

import cv2
import numpy as np
from PIL import Image, ImageCms


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
    """
    width: int = 0
    height: int = 0
    channels: int = 0
    dpi: Tuple[float, float] = (0.0, 0.0)
    color_space: str = ""
    format: str = ""
    has_alpha: bool = False


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
    
    def _extract_metadata(self) -> None:
        """이미지로부터 메타데이터를 추출합니다."""
        if self._data is None:
            return
            
        # 기본 속성 설정
        height, width = self._data.shape[:2]
        channels = 1 if len(self._data.shape) == 2 else self._data.shape[2]
        
        self._metadata = ImageMetadata(
            width=width,
            height=height,
            channels=channels,
            has_alpha=channels == 4,
            color_space='RGBA' if channels == 4 else 'RGB' if channels == 3 else 'L'
        )
        
        # PIL을 사용하여 추가 메타데이터 추출
        try:
            with Image.open(self.filepath) as img:
                self._metadata.format = img.format
                self._metadata.dpi = img.info.get('dpi', (0, 0))
        except Exception:
            pass
    
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
