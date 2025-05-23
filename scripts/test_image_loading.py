#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
이미지 로딩 테스트 스크립트

이 스크립트는 ImageData 클래스의 이미지 로딩 기능을 테스트합니다.
"""

import argparse
import os
import sys
from pathlib import Path

# 프로젝트 루트 디렉토리를 시스템 경로에 추가
project_root = Path(__file__).parent.parent
sys.path.append(str(project_root))

from airphoto_viewer.core.image import ImageData, ImageMetadata

def test_image_loading(image_path: str) -> None:
    """이미지 로딩을 테스트하고 결과를 출력합니다.
    
    Args:
        image_path: 테스트할 이미지 파일 경로
    """
    print(f"\n{'='*50}")
    print(f"이미지 로딩 테스트: {image_path}")
    print("-" * 50)
    
    try:
        # 이미지 로드
        print("이미지를 로드하는 중...")
        img_data = ImageData(image_path)
        
        # 메타데이터 출력
        meta = img_data.metadata
        print("\n[이미지 메타데이터]")
        print(f"  - 크기: {meta.width} x {meta.height}")
        print(f"  - 채널 수: {meta.channels}")
        print(f"  - 색상 공간: {meta.color_space}")
        print(f"  - 포맷: {meta.format}")
        print(f"  - DPI: {meta.dpi}")
        print(f"  - 알파 채널: {'있음' if meta.has_alpha else '없음'}")
        
        # 이미지 데이터 확인
        if img_data.data is not None:
            print("\n[이미지 데이터]")
            print(f"  - 데이터 타입: {img_data.data.dtype}")
            print(f"  - 배열 형태: {img_data.data.shape}")
            
        print("\n✅ 이미지 로딩 테스트가 성공적으로 완료되었습니다.")
        
    except Exception as e:
        print(f"\n❌ 오류 발생: {str(e)}")
        import traceback
        traceback.print_exc()
    
    print("="*50 + "\n")

def main():
    """메인 함수"""
    parser = argparse.ArgumentParser(description='이미지 로딩 테스트 스크립트')
    parser.add_argument('image_path', type=str, help='테스트할 이미지 파일 경로')
    args = parser.parse_args()
    
    # 이미지 경로 확인
    if not os.path.isfile(args.image_path):
        print(f"오류: 파일을 찾을 수 없습니다: {args.image_path}")
        return
    
    # 테스트 실행
    test_image_loading(args.image_path)

if __name__ == "__main__":
    main()
