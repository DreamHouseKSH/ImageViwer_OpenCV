#include "airphoto_viewer/ui/main_window.h"
#include "airphoto_viewer/ui/tile_view.h"

#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QScreen>
#include <QSettings>
#include <QStandardPaths>
#include <QStatusBar>
#include <QToolBar>
#include <QUrl>

namespace airphoto_viewer {
namespace ui {

// Settings keys
const char* const MainWindow::kRecentFilesKey = "recentFiles";
const char* const MainWindow::kGeometryKey = "geometry";
const char* const MainWindow::kWindowStateKey = "windowState";

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_tileView(std::make_unique<TileView>())
    , m_fileMenu(nullptr)
    , m_recentFilesMenu(nullptr)
    , m_viewMenu(nullptr)
    , m_helpMenu(nullptr)
    , m_fileToolBar(nullptr)
    , m_viewToolBar(nullptr)
    , m_openAct(nullptr)
    , m_saveAct(nullptr)
    , m_saveAsAct(nullptr)
    , m_exitAct(nullptr)
    , m_zoomInAct(nullptr)
    , m_zoomOutAct(nullptr)
    , m_zoomOriginalAct(nullptr)
    , m_zoomFitAct(nullptr)
    , m_rotateLeftAct(nullptr)
    , m_rotateRightAct(nullptr)
    , m_resetViewAct(nullptr)
    , m_fullScreenAct(nullptr)
    , m_aboutAct(nullptr)
    , m_aboutQtAct(nullptr)
    , m_recentFileSeparator(nullptr)
    , m_recentFileSubMenuAct(nullptr)
{
    // Set up the UI
    setCentralWidget(m_tileView.get());
    
    // Set up actions, menus, and toolbars
    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    
    // Load settings
    readSettings();
    
    // Update the UI
    updateActions();
    updateWindowTitle();
    
    // Set up drag and drop
    setAcceptDrops(true);
    
    // Set window properties
    setWindowTitle(tr("AirPhoto Viewer"));
    resize(1024, 768);
    
    // Connect signals
    connect(m_tileView.get(), &TileView::viewChanged, this, &MainWindow::updateActions);
    connect(m_tileView.get(), &TileView::imageLoaded, this, &MainWindow::updateWindowTitle);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeEvent(QCloseEvent* event) {
    // Save settings
    writeSettings();
    event->accept();
}

void MainWindow::open() {
    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = QImageReader::supportedMimeTypes();
    for (const QByteArray& mimeTypeName : supportedMimeTypes) {
        mimeTypeFilters.append(mimeTypeName);
    }
    
    // Add the filter for all supported image formats
    QString filter = tr("Image Files (");
    QStringList supportedFormats;
    for (const QByteArray& format : QImageReader::supportedImageFormats()) {
        supportedFormats.append("*." + format);
    }
    filter += supportedFormats.join(" ") + ")";
    
    // Add the "All Files" filter
    filter += ";;" + tr("All Files (*)");
    
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                   QDir::homePath(),
                                                   filter);
    
    if (!fileName.isEmpty()) {
        loadFile(fileName);
    }
}

void MainWindow::openRecentFile() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (action) {
        loadFile(action->data().toString());
    }
}

bool MainWindow::save() {
    if (m_currentFile.isEmpty()) {
        return saveAs();
    } else {
        return saveFile(m_currentFile);
    }
}

bool MainWindow::saveAs() {
    QFileDialog dialog(this, tr("Save As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);
    
    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().constFirst())) {}
    
    return !m_currentFile.isEmpty();
}

void MainWindow::about() {
    QMessageBox::about(this, tr("About AirPhoto Viewer"),
        tr("<b>AirPhoto Viewer</b> is a high-performance image viewer "
           "designed for working with large aerial photographs and maps."
           "<p>Version 1.0.0"
           "<p>Copyright © 2025"));
}

void MainWindow::zoomIn() {
    m_tileView->zoom(1.25);
}

void MainWindow::zoomOut() {
    m_tileView->zoom(0.8);
}

void MainWindow::zoomOriginal() {
    m_tileView->resetView();
}

void MainWindow::zoomFit() {
    m_tileView->fitToView();
}

void MainWindow::toggleFullScreen() {
    if (isFullScreen()) {
        showNormal();
        m_fullScreenAct->setText(tr("&Full Screen"));
        m_fullScreenAct->setStatusTip(tr("Switch to full screen mode"));
    } else {
        showFullScreen();
        m_fullScreenAct->setText(tr("Exit Full Screen"));
        m_fullScreenAct->setStatusTip(tr("Exit full screen mode"));
    }
}

void MainWindow::rotateLeft() {
    // TODO: Implement rotation
    QMessageBox::information(this, tr("Not Implemented"), 
                            tr("Image rotation is not yet implemented."));
}

void MainWindow::rotateRight() {
    // TODO: Implement rotation
    QMessageBox::information(this, tr("Not Implemented"), 
                            tr("Image rotation is not yet implemented."));
}

void MainWindow::resetView() {
    m_tileView->resetView();
}

void MainWindow::createActions() {
    // File menu actions
    m_openAct = new QAction(QIcon::fromTheme("document-open"), tr("&Open..."), this);
    m_openAct->setShortcut(QKeySequence::Open);
    m_openAct->setStatusTip(tr("Open an existing image file"));
    connect(m_openAct, &QAction::triggered, this, &MainWindow::open);
    
    m_saveAct = new QAction(QIcon::fromTheme("document-save"), tr("&Save"), this);
    m_saveAct->setShortcut(QKeySequence::Save);
    m_saveAct->setStatusTip(tr("Save the current image"));
    connect(m_saveAct, &QAction::triggered, this, &MainWindow::save);
    
    m_saveAsAct = new QAction(QIcon::fromTheme("document-save-as"), tr("Save &As..."), this);
    m_saveAsAct->setShortcut(QKeySequence::SaveAs);
    m_saveAsAct->setStatusTip(tr("Save the current image with a new name"));
    connect(m_saveAsAct, &QAction::triggered, this, &MainWindow::saveAs);
    
    m_exitAct = new QAction(tr("E&xit"), this);
    m_exitAct->setShortcut(tr("Ctrl+Q"));
    m_exitAct->setStatusTip(tr("Exit the application"));
    connect(m_exitAct, &QAction::triggered, this, &QWidget::close);

    // View menu actions
    m_zoomInAct = new QAction(QIcon::fromTheme("zoom-in"), tr("Zoom &In"), this);
    m_zoomInAct->setShortcut(QKeySequence::ZoomIn);
    m_zoomInAct->setStatusTip(tr("Zoom in"));
    connect(m_zoomInAct, &QAction::triggered, this, &MainWindow::zoomIn);

    m_zoomOutAct = new QAction(QIcon::fromTheme("zoom-out"), tr("Zoom &Out"), this);
    m_zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    m_zoomOutAct->setStatusTip(tr("Zoom out"));
    connect(m_zoomOutAct, &QAction::triggered, this, &MainWindow::zoomOut);

    m_zoomOriginalAct = new QAction(QIcon::fromTheme("zoom-original"), tr("&Original Size"), this);
    m_zoomOriginalAct->setShortcut(tr("Ctrl+0"));
    m_zoomOriginalAct->setStatusTip(tr("Show the image at its original size"));
    connect(m_zoomOriginalAct, &QAction::triggered, this, &MainWindow::zoomOriginal);

    m_zoomFitAct = new QAction(QIcon::fromTheme("zoom-fit-best"), tr("&Fit to Window"), this);
    m_zoomFitAct->setShortcut(tr("Ctrl+F"));
    m_zoomFitAct->setStatusTip(tr("Fit the image to the window"));
    connect(m_zoomFitAct, &QAction::triggered, this, &MainWindow::zoomFit);

    m_rotateLeftAct = new QAction(QIcon::fromTheme("object-rotate-left"), tr("Rotate &Left"), this);
    m_rotateLeftAct->setShortcut(tr("Ctrl+L"));
    m_rotateLeftAct->setStatusTip(tr("Rotate the image 90 degrees counter-clockwise"));
    connect(m_rotateLeftAct, &QAction::triggered, this, &MainWindow::rotateLeft);

    m_rotateRightAct = new QAction(QIcon::fromTheme("object-rotate-right"), tr("Rotate &Right"), this);
    m_rotateRightAct->setShortcut(tr("Ctrl+R"));
    m_rotateRightAct->setStatusTip(tr("Rotate the image 90 degrees clockwise"));
    connect(m_rotateRightAct, &QAction::triggered, this, &MainWindow::rotateRight);

    m_resetViewAct = new QAction(QIcon::fromTheme("view-refresh"), tr("&Reset View"), this);
    m_resetViewAct->setShortcut(tr("Ctrl+Home"));
    m_resetViewAct->setStatusTip(tr("Reset the view to default"));
    connect(m_resetViewAct, &QAction::triggered, this, &MainWindow::resetView);

    m_fullScreenAct = new QAction(tr("&Full Screen"), this);
    m_fullScreenAct->setShortcut(tr("F11"));
    m_fullScreenAct->setStatusTip(tr("Switch to full screen mode"));
    connect(m_fullScreenAct, &QAction::triggered, this, &MainWindow::toggleFullScreen);

    // Help menu actions
    m_aboutAct = new QAction(tr("&About"), this);
    m_aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(m_aboutAct, &QAction::triggered, this, &MainWindow::about);

    m_aboutQtAct = new QAction(tr("About &Qt"), this);
    m_aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
    connect(m_aboutQtAct, &QAction::triggered, qApp, &QApplication::aboutQt);

    // Recent files actions
    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        connect(m_recentFileActs[i], &QAction::triggered, this, &MainWindow::openRecentFile);
    }

    m_recentFileSeparator = new QAction(this);
    m_recentFileSeparator->setSeparator(true);
    m_recentFileSeparator->setVisible(false);

    updateRecentFileActions();
}

void MainWindow::createMenus() {
    // File menu
    m_fileMenu = menuBar()->addMenu(tr("&File"));
    m_fileMenu->addAction(m_openAct);
    m_fileMenu->addAction(m_saveAct);
    m_fileMenu->addAction(m_saveAsAct);
    m_fileMenu->addSeparator();
    
    // Add recent files menu
    m_recentFilesMenu = m_fileMenu->addMenu(tr("Open &Recent"));
    for (int i = 0; i < MaxRecentFiles; ++i) {
        m_recentFilesMenu->addAction(m_recentFileActs[i]);
    }
    m_recentFilesMenu->setEnabled(hasRecentFiles());
    
    m_fileMenu->addSeparator();
    m_fileMenu->addAction(m_exitAct);

    // View menu
    m_viewMenu = menuBar()->addMenu(tr("&View"));
    m_viewMenu->addAction(m_zoomInAct);
    m_viewMenu->addAction(m_zoomOutAct);
    m_viewMenu->addAction(m_zoomOriginalAct);
    m_viewMenu->addAction(m_zoomFitAct);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_rotateLeftAct);
    m_viewMenu->addAction(m_rotateRightAct);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_resetViewAct);
    m_viewMenu->addSeparator();
    m_viewMenu->addAction(m_fullScreenAct);

    // Help menu
    m_helpMenu = menuBar()->addMenu(tr("&Help"));
    m_helpMenu->addAction(m_aboutAct);
    m_helpMenu->addAction(m_aboutQtAct);
}

void MainWindow::createToolBars() {
    // File toolbar
    m_fileToolBar = addToolBar(tr("File"));
    m_fileToolBar->setObjectName("FileToolBar");
    m_fileToolBar->addAction(m_openAct);
    m_fileToolBar->addAction(m_saveAct);
    m_fileToolBar->addAction(m_saveAsAct);

    // View toolbar
    m_viewToolBar = addToolBar(tr("View"));
    m_viewToolBar->setObjectName("ViewToolBar");
    m_viewToolBar->addAction(m_zoomInAct);
    m_viewToolBar->addAction(m_zoomOutAct);
    m_viewToolBar->addAction(m_zoomOriginalAct);
    m_viewToolBar->addAction(m_zoomFitAct);
    m_viewToolBar->addSeparator();
    m_viewToolBar->addAction(m_rotateLeftAct);
    m_viewToolBar->addAction(m_rotateRightAct);
    m_viewToolBar->addSeparator();
    m_viewToolBar->addAction(m_resetViewAct);
}

void MainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::updateActions() {
    bool hasImage = !m_currentFile.isEmpty();
    
    m_saveAct->setEnabled(hasImage);
    m_saveAsAct->setEnabled(hasImage);
    m_zoomInAct->setEnabled(hasImage);
    m_zoomOutAct->setEnabled(hasImage);
    m_zoomOriginalAct->setEnabled(hasImage);
    m_zoomFitAct->setEnabled(hasImage);
    m_rotateLeftAct->setEnabled(hasImage);
    m_rotateRightAct->setEnabled(hasImage);
    m_resetViewAct->setEnabled(hasImage);
}

void MainWindow::updateWindowTitle() {
    QString title = tr("AirPhoto Viewer");
    if (!m_currentFile.isEmpty()) {
        title = QString("%1 - %2").arg(QFileInfo(m_currentFile).fileName()).arg(title);
    }
    setWindowTitle(title);
}

bool MainWindow::saveFile(const QString& fileName) {
    // TODO: Implement actual saving of the image
    Q_UNUSED(fileName);
    return false;
}

void MainWindow::loadFile(const QString& fileName) {
    qDebug() << "[MainWindow] Loading file:" << fileName;
    
    QFileInfo fileInfo(fileName);
    
    // Check if file exists and is readable
    if (!fileInfo.exists()) {
        qCritical() << "[MainWindow] File does not exist:" << fileName;
        QMessageBox::critical(this, tr("File Not Found"), 
                            tr("The file does not exist:\n%1").arg(fileName));
        return;
    }
    
    if (!fileInfo.isReadable()) {
        qCritical() << "[MainWindow] File is not readable (check permissions):" << fileName;
        QMessageBox::critical(this, tr("Permission Denied"), 
                            tr("Unable to read the file. Please check file permissions.\n%1")
                            .arg(fileName));
        return;
    }
    
    qDebug() << "[MainWindow] File exists and is readable, size:" 
             << fileInfo.size() / (1024.0 * 1024.0) << "MB";
    
    // Check file size to prevent loading extremely large files
    const qint64 maxFileSize = 1024 * 1024 * 500; // 500MB
    if (fileInfo.size() > maxFileSize) {
        qWarning() << "[MainWindow] Large file detected:" << fileInfo.size() / (1024.0 * 1024.0) << "MB";
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Large File"),
                                    tr("The file is very large (%1 MB). This may cause performance issues.\n"
                                       "Do you want to continue?")
                                    .arg(fileInfo.size() / (1024.0 * 1024.0), 0, 'f', 1),
                                    QMessageBox::Yes | QMessageBox::No);
        if (reply != QMessageBox::Yes) {
            return;
        }
    }
    
    // Show loading message
    statusBar()->showMessage(tr("Loading %1...").arg(fileInfo.fileName()));
    QApplication::processEvents();
    
    // Try to load the image
    if (!m_tileView->loadImage(fileName)) {
        QMessageBox::critical(this, tr("Error Loading Image"), 
                            tr("Failed to load the image. The file may be corrupted or in an unsupported format.\n\n"
                               "File: %1")
                            .arg(fileName));
        statusBar()->clearMessage();
        return;
    }
    
    // Update UI
    setCurrentFile(fileName);
    statusBar()->showMessage(tr("Loaded %1").arg(fileInfo.fileName()), 2000);
}

void MainWindow::setCurrentFile(const QString& fileName) {
    m_currentFile = fileName;
    
    // Add to recent files
    QSettings settings;
    QStringList files = settings.value(kRecentFilesKey).toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles) {
        files.removeLast();
    }
    settings.setValue(kRecentFilesKey, files);
    
    updateRecentFileActions();
    updateWindowTitle();
    updateActions();
}

bool MainWindow::hasRecentFiles() const {
    QSettings settings;
    return !settings.value(kRecentFilesKey).toStringList().isEmpty();
}

void MainWindow::initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode) {
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    
    for (const QByteArray &mimeTypeName : supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    dialog.setAcceptMode(acceptMode);
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void MainWindow::updateRecentFileActions() {
    QSettings settings;
    QStringList files = settings.value(kRecentFilesKey).toStringList();
    
    int numRecentFiles = qMin(files.size(), static_cast<int>(MaxRecentFiles));
    
    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(files[i]);
        m_recentFileActs[i]->setVisible(true);
    }
    
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j) {
        m_recentFileActs[j]->setVisible(false);
    }
    
    m_recentFileSeparator->setVisible(numRecentFiles > 0);
}

void MainWindow::readSettings() {
    QSettings settings;
    
    // Restore window geometry and state
    QByteArray geometry = settings.value(kGeometryKey).toByteArray();
    if (!geometry.isEmpty()) {
        restoreGeometry(geometry);
    } else {
        // Default size and position
        resize(1024, 768);
        QScreen* screen = QGuiApplication::primaryScreen();
        if (screen) {
            QRect screenGeometry = screen->availableGeometry();
            move((screenGeometry.width() - width()) / 2,
                 (screenGeometry.height() - height()) / 2);
        }
    }
    
    QByteArray windowState = settings.value(kWindowStateKey).toByteArray();
    if (!windowState.isEmpty()) {
        restoreState(windowState);
    }
    
    // Restore recent files
    updateRecentFileActions();
}

void MainWindow::writeSettings() {
    QSettings settings;
    settings.setValue(kGeometryKey, saveGeometry());
    settings.setValue(kWindowStateKey, saveState());
}

bool MainWindow::event(QEvent* event) {
    // Handle drag and drop
    if (event->type() == QEvent::DragEnter) {
        QDragEnterEvent* dragEvent = static_cast<QDragEnterEvent*>(event);
        if (dragEvent->mimeData()->hasUrls()) {
            QList<QUrl> urls = dragEvent->mimeData()->urls();
            if (!urls.isEmpty() && QFileInfo(urls.first().toLocalFile()).isFile()) {
                dragEvent->acceptProposedAction();
                return true;
            }
        }
    } else if (event->type() == QEvent::Drop) {
        QDropEvent* dropEvent = static_cast<QDropEvent*>(event);
        if (dropEvent->mimeData()->hasUrls()) {
            QList<QUrl> urls = dropEvent->mimeData()->urls();
            if (!urls.isEmpty()) {
                QString fileName = urls.first().toLocalFile();
                if (QFileInfo(fileName).isFile()) {
                    loadFile(fileName);
                    return true;
                }
            }
        }
    }
    
    return QMainWindow::event(event);
}

} // namespace ui
} // namespace airphoto_viewer
