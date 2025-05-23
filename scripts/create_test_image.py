#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
테스트용 샘플 이미지 생성 스크립트
"""

import numpy as np
from PIL import Image, ImageDraw, ImageFont
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
    draw.text(position, text, fill="white", font=font, align="center")
    
    return img

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
        img.save(img_path)
        print(f"생성 완료: {img_path}")

if __name__ == "__main__":
    main()
