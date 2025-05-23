#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
테스트용 샘플 이미지 생성 스크립트
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont, ExifTags
import piexif
from datetime import datetime
import os

def create_sample_image(width=800, height=600, text="테스트 이미지"):
    """테스트용 샘플 이미지를 생성합니다.
    
    Args:
        width: 이미지 너비
        height: 이미지 높이
        text: 이미지에 표시할 텍스트
        
    Returns:
        PIL.Image.Image: 생성된 이미지 객체
    """
    # 그라데이션 배경 생성
    arr = np.zeros((height, width, 3), dtype=np.uint8)
    for y in range(height):
        for x in range(width):
            r = int((x / width) * 255)
            g = int((y / height) * 255)
            b = 128
            arr[y, x] = [r, g, b]
    
    # PIL 이미지로 변환
    img = Image.fromarray(arr)
    draw = ImageDraw.Draw(img)
    
    try:
        # macOS 기본 폰트 사용 시도
        font = ImageFont.truetype("/System/Library/Fonts/Supplemental/Arial.ttf", 40)
    except:
        try:
            # 다른 일반적인 폰트 시도
            font = ImageFont.truetype("Arial.ttf", 40)
        except:
            # 기본 폰트 사용
            font = ImageFont.load_default()
    
    # 텍스트 그리기
    # textbbox를 사용하여 텍스트 크기 계산 (Pillow 10.0.0 이상)
    text_bbox = draw.textbbox((0, 0), text, font=font)
    text_width = text_bbox[2] - text_bbox[0]
    text_height = text_bbox[3] - text_bbox[1]
    position = ((width - text_width) // 2, (height - text_height) // 2)
    
    # 텍스트 그리기
    draw.rectangle(
        [position[0]-10, position[1]-10, 
         position[0]+text_width+10, position[1]+text_height+10],
        fill=(0, 0, 0, 128)
    )
    draw.text(position, text, font=font, fill=(255, 255, 255))
    draw.text(position, text, fill="white", font=font, align="center")
    
    return img

def add_metadata(img_path):
    """이미지에 메타데이터를 추가합니다."""
    # EXIF 데이터 생성
    exif_dict = {
        '0th': {
            piexif.ImageIFD.Make: 'Test Camera',
            piexif.ImageIFD.Model: 'Test Model',
            piexif.ImageIFD.Software: 'Test Image Generator',
        },
        'Exif': {
            piexif.ExifIFD.DateTimeOriginal: datetime.now().strftime("%Y:%m:%d %H:%M:%S"),
            piexif.ExifIFD.FocalLength: (35, 1),  # 35mm
            piexif.ExifIFD.ExposureTime: (1, 125),  # 1/125초
            piexif.ExifIFD.FNumber: (28, 10),  # F/2.8
            piexif.ExifIFD.ISOSpeedRatings: 100,
            piexif.ExifIFD.LensModel: 'Test Lens',
        },
        'GPS': {
            piexif.GPSIFD.GPSVersionID: (2, 2, 0, 0),
            piexif.GPSIFD.GPSLatitudeRef: 'N',
            piexif.GPSIFD.GPSLatitude: [(37, 1), (30, 1), (0, 1)],  # 37.5° N
            piexif.GPSIFD.GPSLongitudeRef: 'E',
            piexif.GPSIFD.GPSLongitude: [(127, 1), (1, 1), (0, 1)],  # 127.0167° E
            piexif.GPSIFD.GPSAltitude: (50, 1),  # 50m
        },
        '1st': {},
        'thumbnail': None
    }
    
    # EXIF 데이터를 바이너리로 변환
    exif_bytes = piexif.dump(exif_dict)
    
    # 이미지에 EXIF 데이터 추가
    piexif.insert(exif_bytes, img_path)

def main():
    """메인 함수"""
    # 테스트 이미지 디렉토리 생성
    os.makedirs("test_images", exist_ok=True)
    
    # 다양한 크기의 테스트 이미지 생성
    sizes = [
        (800, 600, "test_image_800x600.jpg"),
        (1024, 768, "test_image_1024x768.png"),
        (1920, 1080, "test_image_1920x1080.jpg")
    ]
    
    for width, height, filename in sizes:
        img = create_sample_image(width, height, f"{width}x{height} 테스트")
        img_path = os.path.join("test_images", filename)
        
        # 이미지 저장 (일단 임시로 저장)
        temp_path = os.path.join("test_images", "temp_image.jpg")
        img.save(temp_path, "JPEG", quality=95)
        
        # 메타데이터 추가
        add_metadata(temp_path)
        
        # 최종 파일로 이동 (메타데이터가 포함된 상태)
        if os.path.exists(img_path):
            os.remove(img_path)
        os.rename(temp_path, img_path)
        
        print(f"생성 완료: {img_path}")
        print("이미지에 다음 메타데이터가 포함되었습니다:")
        print("- 카메라: Test Camera Test Model")
        print("- 촬영일시: 현재 시간")
        print("- 위치: 37.5° N, 127.0167° E (서울 근방)")
        print("- 고도: 50m")
        print("- 조리개: F/2.8, 셔터속도: 1/125초, ISO: 100")

if __name__ == "__main__":
    main()
