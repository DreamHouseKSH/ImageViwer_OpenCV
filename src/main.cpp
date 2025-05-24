#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QMouseEvent>
#include <QLabel>
#include <QScrollBar>
#include <QDir>
#include <QImage>
#include <QPixmap>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QDebug>
#include <QVBoxLayout>
#include <QImageReader>
#include <QDir>
#include <QLoggingCategory>
#include <QCommandLineParser>
#include <QStackedWidget>
#include <QFileInfo>
#include <QPalette>
#include <QStackedLayout>

#include <opencv2/opencv.hpp>

class ImageViewer : public QMainWindow {
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr) : QMainWindow(parent), scaleFactor(1.0), isPanning(false) {
        // Initialize member variables
        imageLabel = new QLabel();
        welcomeLabel = new QLabel();
        scrollArea = new QScrollArea();
        
        // Configure scroll area
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setWidgetResizable(true);
        scrollArea->setWidget(imageLabel);
        scrollArea->setVisible(true);
        
        // Configure image label
        imageLabel->setBackgroundRole(QPalette::Base);
        imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        imageLabel->setScaledContents(false);
        imageLabel->setAlignment(Qt::AlignCenter);
        
        // Enable mouse tracking for panning
        setMouseTracking(true);
        imageLabel->setMouseTracking(true);
        imageLabel->setAttribute(Qt::WA_TransparentForMouseEvents, false);
        
        // 기본 창 설정
        setWindowTitle("항공사진 뷰어 (C++)");
        resize(1024, 768);

        // 중앙 위젯 설정
        QWidget *centralWidget = new QWidget();
        setCentralWidget(centralWidget);
        
        // 레이아웃 설정
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        
        // 스택 위젯 설정 (웰커메시지와 이미지 뷰어 전환용)
        stackedWidget = new QStackedWidget();
        mainLayout->addWidget(stackedWidget);
        
        // 1. 웰커메시지 화면 설정
        QWidget *welcomeScreen = new QWidget();
        QVBoxLayout *welcomeLayout = new QVBoxLayout(welcomeScreen);
        
        welcomeLabel->setText("<center><h1>항공사진 뷰어</h1>"
                           "<p>이미지를 열려면 '파일 > 열기'를 선택하세요.</p></center>");
        welcomeLabel->setAlignment(Qt::AlignCenter);
        welcomeLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        welcomeLayout->addWidget(welcomeLabel);
        
        // 2. 이미지 뷰어 화면 설정
        QWidget *imageViewer = new QWidget();
        QVBoxLayout *imageViewerLayout = new QVBoxLayout(imageViewer);
        imageViewerLayout->setContentsMargins(0, 0, 0, 0);
        
        imageViewerLayout->addWidget(scrollArea);
        
        // 스택 위젯에 추가
        stackedWidget->addWidget(welcomeScreen);
        stackedWidget->addWidget(imageViewer);
        
        // 메뉴 바 생성
        createActions();
        createMenus();
        
        // 상태 표시줄
        statusBar()->showMessage("이미지를 열려면 '파일 > 열기'를 선택하세요.");
        
        // 초기에는 웰커메시지 화면 표시
        stackedWidget->setCurrentIndex(0);
        qDebug() << "[ImageViewer] Initialized welcome screen";
    }
    
    ~ImageViewer() {}
    
public:
    void loadImage(const QString &fileName) {
        qDebug() << "[ImageViewer] Attempting to load image:" << fileName;
        qDebug() << "[ImageViewer] Current working directory:" << QDir::currentPath();
        qDebug() << "[ImageViewer] File exists:" << QFile::exists(fileName);
        qDebug() << "[ImageViewer] File permissions:" << QFile(fileName).permissions();
        
        // Check if file exists and is readable
        QFileInfo fileInfo(fileName);
        if (!fileInfo.exists()) {
            qCritical() << "[ImageViewer] File does not exist:" << fileName;
            QMessageBox::critical(this, tr("File Not Found"), 
                                tr("The file does not exist:\n%1").arg(fileName));
            return;
        }
        
        if (!fileInfo.isReadable()) {
            qCritical() << "[ImageViewer] File is not readable (check permissions):" << fileName;
            QMessageBox::critical(this, tr("Permission Denied"), 
                                tr("Unable to read the file. Please check file permissions.\n%1")
                                .arg(fileName));
            return;
        }
        
        qDebug() << "[ImageViewer] File exists and is readable, size:" 
                 << fileInfo.size() / (1024.0 * 1024.0) << "MB";
        
        // Try loading with QImageReader first
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        
        qDebug() << "[ImageViewer] Image format:" << reader.format() 
                 << ", size:" << reader.size() 
                 << ", supports animation:" << reader.supportsAnimation();
        
        const QImage newImage = reader.read();
        
        if (newImage.isNull()) {
            qWarning() << "[ImageViewer] QImageReader failed to load image:" << reader.errorString();
            
            // Try loading with OpenCV as fallback
            qDebug() << "[ImageViewer] Trying to load with OpenCV...";
            cv::Mat cvImage = cv::imread(fileName.toStdString(), cv::IMREAD_COLOR);
            
            if (cvImage.empty()) {
                qCritical() << "[ImageViewer] OpenCV failed to load image";
                QMessageBox::warning(this, tr("이미지 뷰어"),
                                   tr("이미지를 로드할 수 없습니다: %1\n\n오류: %2")
                                   .arg(fileName)
                                   .arg(QString::fromStdString(cv::format("OpenCV error: %d", cvImage.empty()))));
                return;
            }
            
            qDebug() << "[ImageViewer] Successfully loaded with OpenCV, size:" 
                     << cvImage.cols << "x" << cvImage.rows 
                     << ", channels:" << cvImage.channels();
            
            // Convert OpenCV image to QImage
            QImage qImage(cvImage.data, cvImage.cols, cvImage.rows, 
                         static_cast<int>(cvImage.step), 
                         QImage::Format_BGR888);
            
            // Make a deep copy since cvImage will be destroyed
            qImage = qImage.copy();
            
            setImage(qImage);
        } else {
            qDebug() << "[ImageViewer] Successfully loaded with QImageReader, size:" 
                     << newImage.size() << ", format:" << newImage.format();
            setImage(newImage);
        }
        
        // Update window title with file name
        setWindowFilePath(fileName);
        
        // Update status bar with image info
        QPixmap currentPixmap = imageLabel->pixmap(Qt::ReturnByValue);
        QByteArray format = QImageReader::imageFormat(fileName);
        
        QString message = tr("\"%1\", %2x%3, %4비트")
            .arg(QDir::toNativeSeparators(fileName))
            .arg(currentPixmap.width())
            .arg(currentPixmap.height())
            .arg(QString(format).toUpper());
            
        qDebug() << "[ImageViewer]" << message;
        statusBar()->showMessage(message);
    }

private slots:
    void open() {
        QString fileName = QFileDialog::getOpenFileName(this,
            tr("이미지 열기"), "",
            tr("이미지 파일 (*.png *.jpg *.jpeg *.bmp *.tif *.tiff);;모든 파일 (*.*)"));
            
        if (!fileName.isEmpty()) {
            loadImage(fileName);
        }
    }
    
    void zoomIn() {
        qDebug() << "[zoomIn] Current scale factor:" << scaleFactor;
        scaleFactor = qMin(scaleFactor * 1.25, 10.0);
        updateImage();
    }
    
    void zoomOut() {
        qDebug() << "[zoomOut] Current scale factor:" << scaleFactor;
        scaleFactor = qMax(scaleFactor * 0.8, 0.1);
        updateImage();
    }
    
    void normalSize() {
        qDebug() << "[normalSize] Resetting to normal size";
        if (!imageLabel->pixmap(Qt::ReturnByValue).isNull()) {
            // Reset to 100% scale
            scaleFactor = 1.0;
            updateImage();
            
            // Update zoom button states
            zoomInAct->setEnabled(true);
            zoomOutAct->setEnabled(true);
        }
    }
    
    void centerImage() {
        if (!imageLabel->pixmap(Qt::ReturnByValue).isNull()) {
            // Center the image in the scroll area
            QSize viewportSize = scrollArea->viewport()->size();
            QSize imageSize = imageLabel->size();
            
            int x = qMax(0, (viewportSize.width() - imageSize.width()) / 2);
            int y = qMax(0, (viewportSize.height() - imageSize.height()) / 2);
            
            imageLabel->move(x, y);
        }
    }
    
    void fitToWindow() {
        bool fitToWindow = fitToWindowAct->isChecked();
        
        if (fitToWindow) {
            // Fit to window
            scrollArea->setWidgetResizable(true);
            imageLabel->setScaledContents(true);
            
            // Disable zoom controls when in fit-to-window mode
            zoomInAct->setEnabled(false);
            zoomOutAct->setEnabled(false);
            normalSizeAct->setEnabled(true);
            
            // Force update the layout
            scrollArea->updateGeometry();
            
            qDebug() << "[fitToWindow] Fitting to window";
        } else {
            // Return to normal size
            scrollArea->setWidgetResizable(false);
            imageLabel->setScaledContents(false);
            normalSize();
        }
        
        updateActions();
    }

private:
    void createActions() {
        // 파일 열기 액션
        openAct = new QAction(tr("열기"), this);
        openAct->setShortcut(QKeySequence::Open);
        connect(openAct, &QAction::triggered, this, &ImageViewer::open);
        
        // 종료 액션
        exitAct = new QAction(tr("종료"), this);
        exitAct->setShortcut(tr("Ctrl+Q"));
        connect(exitAct, &QAction::triggered, this, &QWidget::close);
        
        // 확대 액션
        zoomInAct = new QAction(tr("확대"), this);
        zoomInAct->setShortcut(QKeySequence::ZoomIn);
        zoomInAct->setEnabled(false);
        connect(zoomInAct, &QAction::triggered, this, &ImageViewer::zoomIn);
        
        // 축소 액션
        zoomOutAct = new QAction(tr("축소"), this);
        zoomOutAct->setShortcut(QKeySequence::ZoomOut);
        zoomOutAct->setEnabled(false);
        connect(zoomOutAct, &QAction::triggered, this, &ImageViewer::zoomOut);
        
        // 실제 크기 액션
        normalSizeAct = new QAction(tr("실제 크기"), this);
        normalSizeAct->setShortcut(tr("Ctrl+0"));
        normalSizeAct->setEnabled(false);
        connect(normalSizeAct, &QAction::triggered, this, &ImageViewer::normalSize);
        
        // 창에 맞추기 액션
        fitToWindowAct = new QAction(tr("창에 맞추기"), this);
        fitToWindowAct->setCheckable(true);
        fitToWindowAct->setShortcut(tr("Ctrl+F"));
        fitToWindowAct->setEnabled(false);
        connect(fitToWindowAct, &QAction::triggered, this, &ImageViewer::fitToWindow);
        
        // 디버그 액션들
        debugZoomInAct = new QAction(tr("디버그 확대"), this);
        connect(debugZoomInAct, &QAction::triggered, this, [this]() {
            qDebug() << "[DEBUG] Manual Zoom In";
            scaleFactor = qMin(scaleFactor * 1.25, 10.0);
            updateImage();
        });
        
        debugZoomOutAct = new QAction(tr("디버그 축소"), this);
        connect(debugZoomOutAct, &QAction::triggered, this, [this]() {
            qDebug() << "[DEBUG] Manual Zoom Out";
            scaleFactor = qMax(scaleFactor * 0.8, 0.1);
            updateImage();
        });
        
        debugNormalSizeAct = new QAction(tr("디버그 실제 크기"), this);
        connect(debugNormalSizeAct, &QAction::triggered, this, [this]() {
            qDebug() << "[DEBUG] Manual Normal Size";
            scaleFactor = 1.0;
            updateImage();
        });
    }
    
    void createMenus() {
        // 파일 메뉴
        QMenu *fileMenu = menuBar()->addMenu(tr("파일"));
        fileMenu->addAction(openAct);
        
        // 보기 메뉴
        QMenu *viewMenu = menuBar()->addMenu(tr("보기"));
        viewMenu->addAction(zoomInAct);
        viewMenu->addAction(zoomOutAct);
        viewMenu->addAction(normalSizeAct);
        viewMenu->addSeparator();
        viewMenu->addAction(fitToWindowAct);
        
        // 디버그 메뉴 (개발용)
        QMenu *debugMenu = menuBar()->addMenu(tr("디버그"));
        debugMenu->addAction(debugZoomInAct);
        debugMenu->addAction(debugZoomOutAct);
        debugMenu->addAction(debugNormalSizeAct);
    }
    
    void updateActions() {
        zoomInAct->setEnabled(!fitToWindowAct->isChecked());
        zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
        normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
    }
    
    void adjustScrollBar(QScrollBar *scrollBar, double factor) {
        scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
    }
    
    void updateImage() {
        qDebug() << "[updateImage] Starting update. Image is null:" << image.isNull();
        if (!image.isNull()) {
            qDebug() << "[updateImage] Current scale factor:" << scaleFactor
                     << "Image size:" << image.size();
            // Store current scroll positions
            QScrollBar *hBar = scrollArea->horizontalScrollBar();
            QScrollBar *vBar = scrollArea->verticalScrollBar();
            int hPos = hBar->value();
            int vPos = vBar->value();
            
            // Scale the image while maintaining aspect ratio
            QPixmap pixmap = QPixmap::fromImage(image);
            QSize scaledSize = pixmap.size() * scaleFactor;
            
            // Set the pixmap to the label
            QPixmap scaledPixmap = pixmap.scaled(scaledSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            qDebug() << "[updateImage] Scaled pixmap size:" << scaledPixmap.size()
                     << "Label size:" << imageLabel->size();
            
            imageLabel->setPixmap(scaledPixmap);
            imageLabel->adjustSize();
            
            // Make sure the label and scroll area are properly configured
            imageLabel->setVisible(true);
            scrollArea->setVisible(true);
            scrollArea->setWidget(imageLabel);
            
            // Force update the layout
            scrollArea->updateGeometry();
            updateGeometry();
            
            // Ensure the widget is properly updated
            update();
            repaint();
            QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            
            qDebug() << "[updateImage] After setting pixmap - Label size:" << imageLabel->size()
                     << "Pixmap size:" << imageLabel->pixmap(Qt::ReturnByValue).size();
            
            // Restore scroll positions to maintain view
            hBar->setValue(hPos);
            vBar->setValue(vPos);
            
            // Enable/disable zoom actions based on current scale
            zoomInAct->setEnabled(scaleFactor < 3.0);
            zoomOutAct->setEnabled(scaleFactor > 0.333);
            
            // Center the image if it's smaller than the viewport
            centerImage();
            
            // Update cursor based on panning state
            setCursor(isPanning ? Qt::ClosedHandCursor : Qt::ArrowCursor);
            
            qDebug() << "[updateImage] Image updated. Size:" << imageLabel->size()
                     << "Scale factor:" << scaleFactor;
        }
    }
    
    void setImage(const QImage &newImage) {
        qDebug() << "[setImage] Starting to set new image";
        
        // Check if the image is valid
        if (newImage.isNull()) {
            qWarning() << "[setImage] Error: Invalid image provided (null)";
            return;
        }
        
        try {
            // Store the original image
            image = newImage;
            qDebug() << "[ImageViewer] Image loaded successfully. Size:" << image.size()
                     << "Format:" << image.format()
                     << "Depth:" << image.depth();
            
            // Force update the display
            update();
            QApplication::processEvents();
            
            // Reset scale factor
            scaleFactor = 1.0;
            
            // Update the image
            updateImage();
            
            // Switch to the image view
            if (stackedWidget) {
                stackedWidget->setCurrentIndex(1);
                qDebug() << "[setImage] Switched to image view";
            }
            
            // Update actions
            updateActions();
            
            // Enable actions
            zoomInAct->setEnabled(true);
            zoomOutAct->setEnabled(true);
            normalSizeAct->setEnabled(true);
            fitToWindowAct->setEnabled(true);
            
            qDebug() << "[setImage] Image set successfully. Size:" << image.size()
                     << "Label size:" << imageLabel->size()
                     << "Scale factor:" << scaleFactor;
                     
        } catch (const std::exception &e) {
            qCritical() << "[setImage] Exception:" << e.what();
            QMessageBox::critical(this, tr("Error"), 
                                tr("이미지를 로드하는 중 오류가 발생했습니다.\n%1").arg(e.what()));
        } catch (...) {
            qCritical() << "[setImage] Unknown exception";
            QMessageBox::critical(this, tr("Error"), 
                                tr("이미지를 로드하는 중 알 수 없는 오류가 발생했습니다."));
        }
    }

private:
    // Panning variables
    QPoint lastDragPos;
    bool isPanning;
    
    // Mouse event overrides for panning
    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && !fitToWindowAct->isChecked() && !image.isNull()) {
            isPanning = true;
            lastDragPos = event->pos();
            setCursor(Qt::ClosedHandCursor);
            qDebug() << "[Pan] Start panning at:" << lastDragPos;
        }
        QMainWindow::mousePressEvent(event);
    }
    
    void mouseMoveEvent(QMouseEvent *event) override {
        if (isPanning) {
            QPoint delta = event->pos() - lastDragPos;
            QScrollBar *hBar = scrollArea->horizontalScrollBar();
            QScrollBar *vBar = scrollArea->verticalScrollBar();
            
            hBar->setValue(hBar->value() - delta.x());
            vBar->setValue(vBar->value() - delta.y());
            
            lastDragPos = event->pos();
        }
        QMainWindow::mouseMoveEvent(event);
    }
    
    void mouseReleaseEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && isPanning) {
            isPanning = false;
            setCursor(Qt::ArrowCursor);
            qDebug() << "[Pan] Stopped panning";
        }
        QMainWindow::mouseReleaseEvent(event);
    }
    
    // Debug actions
    QAction *debugZoomInAct;
    QAction *debugZoomOutAct;
    QAction *debugNormalSizeAct;
    
    // UI Elements
    QLabel *imageLabel;
    QLabel *welcomeLabel;
    QScrollArea *scrollArea;
    QStackedWidget *stackedWidget;
    QImage image;
    double scaleFactor;

    // Actions
    QAction *openAct;
    QAction *exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;

    // Menus
    QMenu *fileMenu;
    QMenu *viewMenu;
    
    // Private methods are implemented inline in the class definition
};

int main(int argc, char *argv[]) {
    // Enable debug output
    qSetMessagePattern("[%{type}] %{appname} (%{file}:%{line}): %{message}");
    
    // Set logging rules
    QLoggingCategory::setFilterRules("*.debug=true\nqt.*.debug=false");
    
    QApplication app(argc, argv);
    
    // 애플리케이션 정보 설정
    QCoreApplication::setApplicationName("항공사진 뷰어");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationName("YourCompany");
    
    qDebug() << "Application starting...";
    
    // 메인 윈도우 생성 및 표시
    ImageViewer viewer;
    viewer.show();
    
    // 명령줄 인자로 이미지 파일이 전달된 경우 열기
    if (argc > 1) {
        QString filePath = QString::fromLocal8Bit(argv[1]);
        qDebug() << "Loading image from command line argument:" << filePath;
        viewer.loadImage(filePath);
    } else {
        qDebug() << "No image file specified, starting with empty viewer";
    }
    
    return app.exec();
}

#include "main.moc"


