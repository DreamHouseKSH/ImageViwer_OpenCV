#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
이미지 로딩 및 메타데이터 추출 테스트 스크립트

이 스크립트는 ImageData 클래스의 이미지 로딩 및 메타데이터 추출 기능을 테스트합니다.
"""

import argparse
import json
import os
import sys
from datetime import datetime
from pathlib import Path

# 프로젝트 루트 디렉토리를 시스템 경로에 추가
project_root = Path(__file__).parent.parent
sys.path.append(str(project_root))

from airphoto_viewer.core.image import ImageData, ImageMetadata

def format_bytes(size: int) -> str:
    """바이트 크기를 사람이 읽기 쉬운 형식으로 변환합니다."""
    for unit in ['B', 'KB', 'MB', 'GB', 'TB']:
        if size < 1024:
            return f"{size:.2f} {unit}"
        size /= 1024
    return f"{size:.2f} PB"

def print_section(title: str, data: dict, indent: int = 2) -> None:
    """섹션 제목과 데이터를 포맷에 맞게 출력합니다."""
    indent_str = ' ' * indent
    print(f"\n{'='*50}")
    print(f"{title.upper()}")
    print('-' * 50)
    
    if not data:
        print(f"{indent_str}데이터가 없습니다.")
        return
        
    for key, value in data.items():
        if isinstance(value, dict):
            print(f"{indent_str}{key}:")
            for k, v in value.items():
                print(f"{indent_str*2}{k}: {v}")
        else:
            print(f"{indent_str}{key}: {value}")

def test_image_loading(image_path: str, save_metadata: bool = False) -> None:
    """이미지 로딩 및 메타데이터 추출을 테스트하고 결과를 출력합니다.
    
    Args:
        image_path: 테스트할 이미지 파일 경로
        save_metadata: 메타데이터를 JSON 파일로 저장할지 여부
    """
    print(f"\n{'='*50}")
    print(f"이미지 로딩 및 메타데이터 추출 테스트")
    print(f"{'='*50}")
    print(f"이미지 경로: {image_path}")
    print(f"테스트 시간: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
    
    try:
        # 파일 정보 출력
        file_size = os.path.getsize(image_path)
        file_mtime = datetime.fromtimestamp(os.path.getmtime(image_path))
        
        print("\n[파일 정보]")
        print(f"  - 크기: {format_bytes(file_size)}")
        print(f"  - 수정 시간: {file_mtime}")
        
        # 이미지 로드 및 메타데이터 추출 시간 측정
        import time
        start_time = time.time()
        
        print("\n이미지를 로드하고 메타데이터를 추출하는 중...")
        img_data = ImageData(image_path)
        
        end_time = time.time()
        print(f"로딩 및 추출 완료 (소요 시간: {(end_time - start_time):.3f}초)")
        
        # 기본 메타데이터 출력
        meta = img_data.metadata
        basic_meta = {
            '이미지 크기': f"{meta.width} x {meta.height} 픽셀",
            '색상 채널': f"{meta.channels}개 ({meta.color_space})",
            '이미지 포맷': meta.format,
            '해상도': f"{meta.dpi[0]:.0f} x {meta.dpi[1]:.0f} DPI" if meta.dpi[0] > 0 else "알 수 없음",
            '알파 채널': '있음' if meta.has_alpha else '없음'
        }
        print_section("기본 이미지 정보", basic_meta)
        
        # 카메라 정보 출력
        if meta.camera_make or meta.camera_model:
            camera_info = {
                '제조사': meta.camera_make or '알 수 없음',
                '모델': meta.camera_model or '알 수 없음',
                '촬영 일시': meta.datetime_original or '알 수 없음',
                '노출 시간': f"{meta.exposure_time}초" if meta.exposure_time > 0 else '알 수 없음',
                '조리개 값': f"f/{meta.f_number:.1f}" if meta.f_number > 0 else '알 수 없음',
                'ISO 감도': meta.iso_speed if meta.iso_speed > 0 else '알 수 없음',
                '초점 거리': f"{meta.focal_length}mm" if meta.focal_length > 0 else '알 수 없음'
            }
            print_section("카메라 정보", camera_info)
        
        # GPS 정보 출력
        if meta.gps_latitude != 0 or meta.gps_longitude != 0:
            gps_info = {
                '위도': f"{abs(meta.gps_latitude):.6f}° {'N' if meta.gps_latitude >= 0 else 'S'}" if meta.gps_latitude != 0 else '알 수 없음',
                '경도': f"{abs(meta.gps_longitude):.6f}° {'E' if meta.gps_longitude >= 0 else 'W'}" if meta.gps_longitude != 0 else '알 수 없음',
                '고도': f"{meta.gps_altitude:.2f} m" if meta.gps_altitude != 0 else '알 수 없음'
            }
            print_section("위치 정보", gps_info)
        
        # 색상 프로파일 정보 출력
        if meta.color_profile:
            color_profile = {
                '프로파일 유형': meta.color_profile.get('profile_type', '알 수 없음'),
                '크기': format_bytes(meta.color_profile.get('profile_size', 0))
            }
            print_section("색상 프로파일", color_profile)
        
        # EXIF 데이터 요약 출력 (처음 10개 항목만)
        if meta.exif_data:
            exif_summary = {k: v for i, (k, v) in enumerate(meta.exif_data.items()) if i < 10}
            print_section("EXIF 데이터 (일부)", exif_summary)
            print(f"  ... (총 {len(meta.exif_data)}개 항목 중 10개만 표시됨)")
        
        # 메타데이터를 JSON 파일로 저장
        if save_metadata:
            output_path = f"{os.path.splitext(image_path)[0]}_metadata.json"
            with open(output_path, 'w', encoding='utf-8') as f:
                json.dump(meta.to_dict(), f, ensure_ascii=False, indent=2)
            print(f"\n[완료] 메타데이터가 다음 파일에 저장되었습니다: {output_path}")
        
    except Exception as e:
        import traceback
        print(f"\n[오류 발생] {str(e)}")
        print("\n상세 오류 추적:")
        traceback.print_exc()

def main():
    """메인 함수"""
    parser = argparse.ArgumentParser(description='이미지 로딩 및 메타데이터 추출 테스트')
    parser.add_argument('image_path', type=str, help='테스트할 이미지 파일 경로')
    parser.add_argument('--save', action='store_true', help='메타데이터를 JSON 파일로 저장')
    args = parser.parse_args()
    
    # 이미지 경로 확인
    if not os.path.isfile(args.image_path):
        print(f"오류: 파일을 찾을 수 없습니다: {args.image_path}")
        return
    
    # 테스트 실행
    test_image_loading(args.image_path, args.save)

if __name__ == "__main__":
    main()
