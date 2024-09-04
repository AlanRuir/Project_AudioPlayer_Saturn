#include <QGuiApplication>
#include <QApplication>
#include <QIcon>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <yaml-cpp/yaml.h>
#include "audio_player_controller.h"
#include "audio_display_widget.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AudioPlayer");
    QIcon appIcon;
    appIcon.addFile(":/orange_vision_logo.png", QSize(), QIcon::Normal, QIcon::Off);
    app.setWindowIcon(appIcon);

    // 加载QSS样式表
    QFile file(":/styles.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) 
    {
        QTextStream stream(&file);
        QString stylesheet = stream.readAll();
        app.setStyleSheet(stylesheet);
        file.close();
    } 
    else 
    {
        qWarning("Could not open styles.qss file");
    }

    AudioDisplayWidget widget;
    AudioPlayerController controller;
    controller.play();

    widget.setWindowTitle("视觉橙音乐播放器");
    widget.setWindowFlags(Qt::Window | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    // 设置窗口为始终置顶
    widget.setWindowFlags(widget.windowFlags() | Qt::WindowStaysOnTopHint);

    QObject::connect(&widget, &AudioDisplayWidget::GetMp3FileName, &controller, &AudioPlayerController::GetMp3FileName);

    widget.show();
    widget.setFocus();

    return app.exec();
}