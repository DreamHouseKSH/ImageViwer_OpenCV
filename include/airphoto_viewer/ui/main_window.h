#ifndef AIRPHOTO_VIEWER_MAIN_WINDOW_H
#define AIRPHOTO_VIEWER_MAIN_WINDOW_H

#include <QMainWindow>
#include <QString>
#include <QFileDialog>
#include <memory>

// Forward declarations
class QAction;
class QMenu;
class QToolBar;

namespace airphoto_viewer {
namespace ui {

class TileView;

/**
 * @brief The main window of the AirPhoto Viewer application
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;
    bool event(QEvent* event) override;

private slots:
    // File menu actions
    void open();
    void openRecentFile();
    bool save();
    bool saveAs();
    void about();
    
    // View actions
    void zoomIn();
    void zoomOut();
    void zoomOriginal();
    void zoomFit();
    void toggleFullScreen();
    
    // Image actions
    void rotateLeft();
    void rotateRight();
    void resetView();
    
    // Update UI based on current state
    void updateActions();
    void updateWindowTitle();

    // Helper functions
    bool saveFile(const QString& fileName);
    void setCurrentFile(const QString& fileName);
    void updateRecentFileActions();
    void readSettings();
    void writeSettings();
    bool hasRecentFiles() const;
    void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode);
    
public:
    // File operations
    void loadFile(const QString& fileName);
    
private:
    // UI creation
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    
    // Member variables
    std::unique_ptr<TileView> m_tileView;
    QString m_currentFile;
    
    // UI elements
    QMenu* m_fileMenu;
    QMenu* m_recentFilesMenu;
    QMenu* m_viewMenu;
    QMenu* m_helpMenu;
    
    QToolBar* m_fileToolBar;
    QToolBar* m_viewToolBar;
    
    // Actions
    QAction* m_openAct;
    QAction* m_saveAct;
    QAction* m_saveAsAct;
    QAction* m_exitAct;
    
    QAction* m_zoomInAct;
    QAction* m_zoomOutAct;
    QAction* m_zoomOriginalAct;
    QAction* m_zoomFitAct;
    QAction* m_rotateLeftAct;
    QAction* m_rotateRightAct;
    QAction* m_resetViewAct;
    QAction* m_fullScreenAct;
    
    QAction* m_aboutAct;
    QAction* m_aboutQtAct;
    
    // Recent files
    enum { MaxRecentFiles = 5 };
    QAction* m_recentFileActs[MaxRecentFiles];
    QAction* m_recentFileSeparator;
    QAction* m_recentFileSubMenuAct;
    
    // Settings keys
    static const char* const kRecentFilesKey;
    static const char* const kGeometryKey;
    static const char* const kWindowStateKey;
};

} // namespace ui
} // namespace airphoto_viewer

#endif // AIRPHOTO_VIEWER_MAIN_WINDOW_H
