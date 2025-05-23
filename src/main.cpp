#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QImage>
#include <QPixmap>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QStatusBar>
#include <QDebug>

#include <opencv2/opencv.hpp>

class ImageViewer : public QMainWindow {
    Q_OBJECT

public:
    ImageViewer(QWidget *parent = nullptr) : QMainWindow(parent) {
        // 기본 창 설정
        setWindowTitle("항공사진 뷰어 (C++)");
        resize(1024, 768);

        // 중앙 위젯 설정
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // 레이아웃 설정
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        layout->setContentsMargins(0, 0, 0, 0);
        
        // 스크롤 영역 설정
        scrollArea = new QScrollArea(this);
        scrollArea->setBackgroundRole(QPalette::Dark);
        scrollArea->setAlignment(Qt::AlignCenter);
        
        // 이미지 레이블 설정
        imageLabel = new QLabel(this);
        imageLabel->setBackgroundRole(QPalette::Base);
        imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        imageLabel->setScaledContents(false);
        
        scrollArea->setWidget(imageLabel);
        layout->addWidget(scrollArea);
        
        // 메뉴 바 생성
        createActions();
        createMenus();
        
        // 상태 표시줄
        statusBar()->showMessage("이미지를 열려면 '파일 > 열기'를 선택하세요.");
        
        // 초기 화면에 표시할 메시지
        QLabel *welcomeLabel = new QLabel("<center><h1>항공사진 뷰어</h1>"
                                         "<p>이미지를 열려면 '파일 > 열기'를 선택하세요.</p></center>");
        welcomeLabel->setAlignment(Qt::AlignCenter);
        setCentralWidget(welcomeLabel);
    }
    
    ~ImageViewer() {}

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
        scaleImage(1.25);
    }
    
    void zoomOut() {
        scaleImage(0.8);
    }
    
    void normalSize() {
        imageLabel->adjustSize();
        scaleFactor = 1.0;
    }
    
    void fitToWindow() {
        bool fitToWindow = fitToWindowAct->isChecked();
        scrollArea->setWidgetResizable(fitToWindow);
        if (!fitToWindow) {
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
        
        // 확대/축소 액션
        zoomInAct = new QAction(tr("확대 (25%)"), this);
        zoomInAct->setShortcut(QKeySequence::ZoomIn);
        zoomInAct->setEnabled(false);
        connect(zoomInAct, &QAction::triggered, this, &ImageViewer::zoomIn);
        
        zoomOutAct = new QAction(tr("축소 (25%)"), this);
        zoomOutAct->setShortcut(QKeySequence::ZoomOut);
        zoomOutAct->setEnabled(false);
        connect(zoomOutAct, &QAction::triggered, this, &ImageViewer::zoomOut);
        
        normalSizeAct = new QAction(tr("원본 크기"), this);
        normalSizeAct->setShortcut(tr("Ctrl+0"));
        normalSizeAct->setEnabled(false);
        connect(normalSizeAct, &QAction::triggered, this, &ImageViewer::normalSize);
        
        fitToWindowAct = new QAction(tr("창에 맞추기"), this);
        fitToWindowAct->setEnabled(false);
        fitToWindowAct->setCheckable(true);
        fitToWindowAct->setShortcut(tr("Ctrl+F"));
        connect(fitToWindowAct, &QAction::triggered, this, &ImageViewer::fitToWindow);
    }
    
    void createMenus() {
        // 파일 메뉴
        fileMenu = menuBar()->addMenu(tr("파일"));
        fileMenu->addAction(openAct);
        fileMenu->addSeparator();
        fileMenu->addAction(exitAct);
        
        // 보기 메뉴
        viewMenu = menuBar()->addMenu(tr("보기"));
        viewMenu->addAction(zoomInAct);
        viewMenu->addAction(zoomOutAct);
        viewMenu->addAction(normalSizeAct);
        viewMenu->addSeparator();
        viewMenu->addAction(fitToWindowAct);
    }
    
    void updateActions() {
        zoomInAct->setEnabled(!fitToWindowAct->isChecked());
        zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
        normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
    }
    
    void scaleImage(double factor) {
        scaleFactor *= factor;
        imageLabel->resize(scaleFactor * imageLabel->pixmap(Qt::ReturnByValue).size());
        
        adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
        adjustScrollBar(scrollArea->verticalScrollBar(), factor);
        
        zoomInAct->setEnabled(scaleFactor < 3.0);
        zoomOutAct->setEnabled(scaleFactor > 0.333);
    }
    
    void adjustScrollBar(QScrollBar *scrollBar, double factor) {
        scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
    }
    
    void loadImage(const QString &fileName) {
        QImageReader reader(fileName);
        reader.setAutoTransform(true);
        const QImage newImage = reader.read();
        
        if (newImage.isNull()) {
            // OpenCV를 사용하여 이미지 로드 시도
            cv::Mat cvImage = cv::imread(fileName.toStdString());
            if (cvImage.empty()) {
                QMessageBox::warning(this, tr("이미지 뷰어"),
                                   tr("이미지를 로드할 수 없습니다: %1")
                                   .arg(reader.errorString()));
                return;
            }
            
            // OpenCV 이미지를 QImage로 변환
            QImage qImage(cvImage.data, cvImage.cols, cvImage.rows, 
                         static_cast<int>(cvImage.step), 
                         QImage::Format_BGR888);
            
            setImage(qImage);
        } else {
            setImage(newImage);
        }
        
        setWindowFilePath(fileName);
        
        const QString message = tr("\"%1\", %2x%3, %4비트")
            .arg(QDir::toNativeSeparators(fileName))
            .arg(imageLabel->pixmap(Qt::ReturnByValue).width())
            .arg(imageLabel->pixmap(Qt::ReturnByValue).height())
            .arg(QImageReader::imageFormat(fileName).toUpper());
        statusBar()->showMessage(message);
    }
    
    void setImage(const QImage &newImage) {
        image = newImage;
        imageLabel->setPixmap(QPixmap::fromImage(image));
        
        scaleFactor = 1.0;
        
        scrollArea->setVisible(true);
        fitToWindowAct->setEnabled(true);
        updateActions();
        
        if (!fitToWindowAct->isChecked()) {
            imageLabel->adjustSize();
        }
    }
    
    QLabel *imageLabel;
    QScrollArea *scrollArea;
    double scaleFactor = 1.0;
    QImage image;
    
    // 메뉴 관련 멤버 변수
    QMenu *fileMenu;
    QMenu *viewMenu;
    QAction *openAct;
    QAction *exitAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 애플리케이션 정보 설정
    QCoreApplication::setApplicationName("항공사진 뷰어");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationName("YourCompany");
    
    // 메인 윈도우 생성 및 표시
    ImageViewer viewer;
    viewer.show();
    
    // 명령줄 인자로 이미지 파일이 전달된 경우 열기
    if (argc > 1) {
        viewer.loadImage(QString::fromLocal8Bit(argv[1]));
    }
    
    return app.exec();
}

#include "main.moc"
