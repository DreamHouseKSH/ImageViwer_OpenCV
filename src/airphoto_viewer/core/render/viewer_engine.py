#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
이미지 뷰어 엔진 모듈입니다.
PyQt6를 사용하여 이미지를 표시하는 기능을 제공합니다.
"""

import os
import sys
import numpy as np
from typing import Optional, Tuple
from dataclasses import dataclass

from PyQt6.QtWidgets import (QApplication, QMainWindow, QLabel, QScrollArea, 
                           QVBoxLayout, QWidget, QSizePolicy, QFileDialog, QStatusBar)
from PyQt6.QtGui import QPixmap, QImage, QAction, QKeySequence, QPainter
from PyQt6.QtCore import Qt, QSize, QRectF, pyqtSignal, QPoint, QPointF
from PyQt6.QtWidgets import QGraphicsView, QGraphicsScene, QGraphicsPixmapItem

from ..image.image_data import ImageData

@dataclass
class ImageViewerState:
    """이미지 뷰어의 현재 상태를 저장하는 데이터 클래스"""
    scale_factor: float = 1.0
    rotation: float = 0.0
    is_flipped_h: bool = False
    is_flipped_v: bool = False
    last_mouse_pos: Optional[QPoint] = None

class ImageViewer(QMainWindow):
    """이미지를 표시하고 기본적인 조작을 제공하는 뷰어 클래스"""
    
    def __init__(self, parent=None):
        super().__init__(parent)
        
        # 기본 창 설정
        self.setWindowTitle("항공사진 뷰어")
        self.setMinimumSize(800, 600)
        
        # 상태 초기화
        self.state = ImageViewerState()
        self.image_data = None
        self.pixmap_item = None
        
        # UI 초기화
        self.init_ui()
        
        # 상태 표시줄
        self.statusBar().showMessage("이미지를 불러오려면 '파일 > 열기'를 선택하세요.")
    
    def init_ui(self):
        """사용자 인터페이스 초기화"""
        # 중앙 위젯과 레이아웃 설정
        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        
        # 그래픽 뷰와 씬 설정
        self.scene = QGraphicsScene(self)
        self.view = QGraphicsView(self.scene)
        self.view.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
        self.view.setDragMode(QGraphicsView.DragMode.ScrollHandDrag)
        self.view.setTransformationAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.view.setResizeAnchor(QGraphicsView.ViewportAnchor.AnchorUnderMouse)
        self.view.setHorizontalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self.view.setVerticalScrollBarPolicy(Qt.ScrollBarPolicy.ScrollBarAsNeeded)
        self.view.setFrameShape(QGraphicsView.Shape.NoFrame)
        
        # 레이아웃 설정
        layout = QVBoxLayout(self.central_widget)
        layout.setContentsMargins(0, 0, 0, 0)
        layout.addWidget(self.view)
        
        # 메뉴 바 설정
        self.create_menus()
        
        # 상태 표시줄
        self.status_bar = self.statusBar()
    
    def create_menus(self):
        """메뉴 바 생성"""
        menubar = self.menuBar()
        
        # 파일 메뉴
        file_menu = menubar.addMenu("파일")
        
        # 열기 액션
        open_action = QAction("열기", self)
        open_action.setShortcut(QKeySequence.StandardKey.Open)
        open_action.triggered.connect(self.open_image)
        file_menu.addAction(open_action)
        
        # 종료 액션
        exit_action = QAction("종료", self)
        exit_action.setShortcut(QKeySequence.StandardKey.Quit)
        exit_action.triggered.connect(self.close)
        file_menu.addAction(exit_action)
        
        # 보기 메뉴
        view_menu = menubar.addMenu("보기")
        
        # 확대/축소 액션
        zoom_in_action = QAction("확대 (25%)", self)
        zoom_in_action.setShortcut(QKeySequence.StandardKey.ZoomIn)
        zoom_in_action.triggered.connect(self.zoom_in)
        view_menu.addAction(zoom_in_action)
        
        zoom_out_action = QAction("축소 (25%)", self)
        zoom_out_action.setShortcut(QKeySequence.StandardKey.ZoomOut)
        zoom_out_action.triggered.connect(self.zoom_out)
        view_menu.addAction(zoom_out_action)
        
        normal_size_action = QAction("원본 크기 (100%)", self)
        normal_size_action.setShortcut("Ctrl+0")
        normal_size_action.triggered.connect(self.normal_size)
        view_menu.addAction(normal_size_action)
        
        view_menu.addSeparator()
        
        # 이미지 맞추기 액션
        fit_to_window_action = QAction("창에 맞추기", self)
        fit_to_window_action.triggered.connect(self.fit_to_window)
        view_menu.addAction(fit_to_window_action)
    
    def open_image(self):
        """이미지 파일 열기"""
        file_name, _ = QFileDialog.getOpenFileName(
            self, 
            "이미지 열기", 
            "", 
            "이미지 파일 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;모든 파일 (*.*)"
        )
        
        if file_name:
            self.load_image(file_name)
    
    def load_image(self, file_path: str):
        """이미지 파일을 로드하여 표시"""
        try:
            # 이미지 데이터 로드
            self.image_data = ImageData()
            self.image_data.load(file_path)
            
            # OpenCV 이미지를 QImage로 변환
            height, width, channel = self.image_data.data.shape
            bytes_per_line = 3 * width
            
            # BGR을 RGB로 변환 (OpenCV는 BGR, QImage는 RGB 사용)
            rgb_image = self.image_data.data[..., ::-1]  # BGR to RGB
            
            qimage = QImage(
                rgb_image.data, 
                width, 
                height, 
                bytes_per_line, 
                QImage.Format.Format_RGB888
            )
            
            # QPixmap으로 변환
            pixmap = QPixmap.fromImage(qimage)
            
            # 기존 씬 정리
            self.scene.clear()
            
            # 새 픽스맵 아이템 생성 및 추가
            self.pixmap_item = QGraphicsPixmapItem(pixmap)
            self.scene.addItem(self.pixmap_item)
            
            # 뷰 리셋
            self.view.resetTransform()
            self.state.scale_factor = 1.0
            self.state.rotation = 0.0
            
            # 상태 표시줄 업데이트
            self.status_bar.showMessage(f"로드 완료: {os.path.basename(file_path)} ({width}x{height})")
            
            # 창에 맞게 조정
            self.fit_to_window()
            
        except Exception as e:
            self.status_bar.showMessage(f"오류: {str(e)}")
    
    def wheelEvent(self, event):
        """마우스 휠 이벤트 핸들러 (줌 기능)"""
        if self.pixmap_item is None:
            return
            
        # 휠 델타에 따라 확대/축소
        zoom_factor = 1.15  # 15% 씩 확대/축소
        
        if event.angleDelta().y() > 0:
            # 확대
            self.view.scale(zoom_factor, zoom_factor)
            self.state.scale_factor *= zoom_factor
        else:
            # 축소
            self.view.scale(1.0 / zoom_factor, 1.0 / zoom_factor)
            self.state.scale_factor /= zoom_factor
        
        self.update_status_bar()
    
    def zoom_in(self):
        """이미지 확대"""
        if self.pixmap_item:
            self.view.scale(1.25, 1.25)
            self.state.scale_factor *= 1.25
            self.update_status_bar()
    
    def zoom_out(self):
        """이미지 축소"""
        if self.pixmap_item:
            self.view.scale(0.8, 0.8)
            self.state.scale_factor *= 0.8
            self.update_status_bar()
    
    def normal_size(self):
        """이미지를 원본 크기로 표시"""
        if self.pixmap_item:
            self.view.resetTransform()
            self.state.scale_factor = 1.0
            self.update_status_bar()
    
    def fit_to_window(self):
        """이미지를 창에 맞게 조정"""
        if self.pixmap_item:
            # 뷰포트 크기 가져오기
            view_rect = self.view.viewport().rect()
            view_size = view_rect.size()
            
            # 이미지 크기 가져오기
            pixmap_size = self.pixmap_item.pixmap().size()
            
            # 종횡비 유지하며 맞출 스케일 계산
            scale_x = view_size.width() / pixmap_size.width()
            scale_y = view_size.height() / pixmap_size.height()
            scale = min(scale_x, scale_y) * 0.95  # 약간의 여백 추가
            
            # 변환 적용
            self.view.resetTransform()
            self.view.scale(scale, scale)
            self.state.scale_factor = scale
            
            # 이미지를 뷰포트 중앙에 배치
            self.view.centerOn(self.pixmap_item)
            
            self.update_status_bar()
    
    def update_status_bar(self):
        """상태 표시줄 업데이트"""
        if self.pixmap_item:
            zoom_percent = int(self.state.scale_factor * 100)
            self.status_bar.showMessage(f"확대율: {zoom_percent}% | 회전: {int(self.state.rotation)}°")

def main():
    """애플리케이션 진입점"""
    app = QApplication(sys.argv)
    
    # 애플리케이션 스타일 설정
    app.setStyle('Fusion')
    
    # 뷰어 생성 및 표시
    viewer = ImageViewer()
    viewer.show()
    
    # 커맨드 라인 인자가 있으면 첫 번째 인자를 이미지 파일로 로드
    if len(sys.argv) > 1 and os.path.isfile(sys.argv[1]):
        viewer.load_image(sys.argv[1])
    
    sys.exit(app.exec())

if __name__ == "__main__":
    main()
