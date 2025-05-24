/**
 * @file test_main_window.cpp
 * @brief MainWindow 클래스의 기본 기능을 테스트하기 위한 프로그램
 */

#include <QTest>
#include <QSignalSpy>
#include <QTestEventList>
#include <QDebug>
#include <QMenu>
#include <QToolBar>

// 테스트할 클래스 헤더
#include "airphoto_viewer/ui/main_window.h"

using namespace airphoto_viewer::ui;

/**
 * @brief MainWindow 클래스의 단위 테스트
 */
class TestMainWindow : public QObject
{
    Q_OBJECT

private slots:
    /**
     * @brief 테스트 초기화
     */
    void initTestCase()
    {
        qDebug() << "테스트 케이스 초기화";
    }

    /**
     * @brief MainWindow 생성 테스트
     */
    void testWindowCreation()
    {
        MainWindow window;
        QVERIFY(window.windowTitle().contains("AirPhoto"));
    }

    /**
     * @brief 기본 UI 요소 존재 여부 테스트
     */
    void testUIElements()
    {
        MainWindow window;
        
        // 기본 메뉴 확인
        QMenu* fileMenu = window.findChild<QMenu*>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(fileMenu != nullptr);
        
        // 툴바 확인
        QList<QToolBar*> toolBars = window.findChildren<QToolBar*>();
        QVERIFY(!toolBars.isEmpty());
        
        // 기본 동작 확인
        QVERIFY(window.windowTitle().contains("AirPhoto"));
    }

    /**
     * @brief 테스트 정리
     */
    void cleanupTestCase()
    {
        qDebug() << "테스트 케이스 정리";
    }
};

QTEST_MAIN(TestMainWindow)
#include "test_main_window.moc"
