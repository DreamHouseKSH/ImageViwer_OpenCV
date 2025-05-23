"""
항공사진 뷰어의 핵심 기능을 제공하는 모듈입니다.

이 모듈은 이미지 처리, 타일 생성, 렌더링 등 핵심 기능을 제공합니다.
"""

from pathlib import Path
from typing import Optional

# 버전 정보
__version__ = "0.1.0"

# 모듈 초기화
def init(cache_dir: Optional[Path] = None) -> None:
    """모듈을 초기화합니다.
    
    Args:
        cache_dir: 캐시 디렉토리 경로. None인 경우 기본 경로 사용
    """
    # 필요한 초기화 로직 추가
    pass
