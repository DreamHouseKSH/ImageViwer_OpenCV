/**
 * @file main.cpp
 * @brief 항공사진 뷰어 애플리케이션의 진입점
 * 
 * 이 파일은 애플리케이션의 진입점을 포함하며, 애플리케이션의
 * 기본 설정 및 초기화를 담당합니다.
 */

#include <QApplication>
#include <QMessageBox>
#include <QCommandLineParser>
#include <QDebug>
#include <QLoggingCategory>

// 메인 윈도우 헤더
#include "airphoto_viewer/ui/main_window.h"

// 로깅 카테고리 정의
Q_LOGGING_CATEGORY(mainLog, "airphoto.main")

/**
 * @brief 애플리케이션 진입점
 * 
 * @param argc 명령줄 인수의 수
 * @param argv 명령줄 인수 배열
 * @return int 애플리케이션 종료 코드
 */
int main(int argc, char *argv[]) {
    // 애플리케이션 초기화
    QApplication app(argc, argv);
    
    // 애플리케이션 메타데이터 설정
    QCoreApplication::setApplicationName("AirPhotoViewer");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("AirPhotoTeam");
    
    // 로깅 설정
    QLoggingCategory::setFilterRules("airphoto.*=true");
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz} %{if-category}%{category}: %{endif}%{type}] %{message}");
    
    qCDebug(mainLog) << "애플리케이션 시작";
    
    // 명령줄 인수 파싱
    QCommandLineParser parser;
    parser.setApplicationDescription("항공사진 뷰어 애플리케이션");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("파일", "열 이미지 파일");
    
    // 명령줄 옵션 추가
    QCommandLineOption debugOption("d", "디버그 모드 활성화");
    parser.addOption(debugOption);
    
    // 파싱 실행
    parser.process(app);
    
    // 디버그 모드 설정
    bool debugMode = parser.isSet(debugOption);
    if (debugMode) {
        qCDebug(mainLog) << "디버그 모드 활성화";
    }
    
    try {
        // 메인 윈도우 생성
        airphoto_viewer::ui::MainWindow mainWindow;
        mainWindow.show();
        
        // 명령줄에서 파일 열기
        const QStringList args = parser.positionalArguments();
        if (!args.isEmpty()) {
            mainWindow.loadFile(args.first());
        }
        
        // 애플리케이션 이벤트 루프 실행
        return app.exec();
        
    } catch (const std::exception &e) {
        qCCritical(mainLog) << "치명적 오류 발생:" << e.what();
        QMessageBox::critical(nullptr, 
                            "치명적 오류", 
                            QString("치명적 오류가 발생하여 애플리케이션이 종료됩니다.\n\n오류: %1").arg(e.what()));
        return 1;
    } catch (...) {
        qCCritical(mainLog) << "알 수 없는 치명적 오류 발생";
        QMessageBox::critical(nullptr, 
                            "치명적 오류", 
                            "알 수 없는 치명적 오류가 발생하여 애플리케이션이 종료됩니다.");
        return 1;
    }
}

#include "main.moc"
