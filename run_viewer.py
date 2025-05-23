#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
항공사진 뷰어 실행 스크립트
"""

import sys
import os

# 프로젝트 루트 디렉토리를 시스템 경로에 추가
sys.path.insert(0, os.path.abspath('.'))

# 뷰어 엔진 임포트
from src.airphoto_viewer.core.render.viewer_engine import main

if __name__ == "__main__":
    main()
