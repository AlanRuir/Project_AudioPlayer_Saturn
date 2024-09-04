#include "audio_display_widget.h"
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QTransform>
#include <QPainterPath>
#include <QFileInfo>

AudioDisplayWidget::AudioDisplayWidget(QWidget *parent)
    : QWidget(parent)
    , label_music_icon(new QLabel(this))
    , label_music_info(new QLabel(this))
    , label_music_author(new QLabel(this))
    , file_dialog(new QFileDialog(this))
    , button_open_mp3(new QPushButton(this))
    , rotation_angle(0)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *audio_display_layout = new QVBoxLayout();

    QPixmap pixmap(":/orange_vision_logo.png");
    circular_pixmap = createCircularPixmap(pixmap).scaled(50, 50, Qt::KeepAspectRatio);
    label_music_icon->setFixedSize(circular_pixmap.size());
    label_music_icon->setPixmap(circular_pixmap);

    label_music_icon->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    audio_display_layout->addWidget(label_music_icon);

    QVBoxLayout *audio_info_layout = new QVBoxLayout();
    label_music_info->setAlignment(Qt::AlignHCenter);
    label_music_info->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label_music_info->setText("橙意音乐");
    label_music_author->setAlignment(Qt::AlignHCenter);
    label_music_author->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    label_music_author->setText("张瑞强");
    audio_info_layout->addWidget(label_music_info);
    audio_info_layout->addWidget(label_music_author);

    QVBoxLayout *button_layout = new QVBoxLayout();
    button_open_mp3->setText("打开音乐");
    button_open_mp3->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    connect(button_open_mp3, &QPushButton::clicked, this, &AudioDisplayWidget::onOpenMp3ButtonClicked);
    button_layout->addWidget(button_open_mp3);

    QHBoxLayout *main_layout = new QHBoxLayout(this);
    main_layout->addLayout(audio_display_layout, 1);
    main_layout->addLayout(audio_info_layout, 2);
    main_layout->addLayout(button_layout, 1);

    setLayout(main_layout);

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &AudioDisplayWidget::rotateIcon);
    timer->start(30); // Rotate every 30 ms
}

void AudioDisplayWidget::onOpenMp3ButtonClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open File"),
                                                    QDir::homePath(),
                                                    tr("mp3 Files (*.mp3);;MP3 Files (*.MP3)"));
    if (!fileName.isEmpty()) 
    {
        emit GetMp3FileName(fileName);
        // label_music_info->setText(fileName);

        // 使用 QFileInfo 获取文件名
        QFileInfo fileInfo(fileName);
        QString fileBaseName = fileInfo.fileName();
        
        // 仅显示文件名
        label_music_info->setText(fileBaseName);
    }
}

void AudioDisplayWidget::rotateIcon()
{
    rotation_angle += 5; // Rotate 5 degrees each time
    if (rotation_angle >= 360)
    {
        rotation_angle = 0;   
    }

    QTransform transform;
    transform.rotate(rotation_angle);

    // 将图像放置在透明的正方形背景中，以保持中心位置
    QPixmap rotated_pixmap(label_music_icon->size());
    rotated_pixmap.fill(Qt::transparent);  // 填充为透明背景

    QPainter painter(&rotated_pixmap);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.translate(rotated_pixmap.width() / 2, rotated_pixmap.height() / 2);
    painter.rotate(rotation_angle);
    painter.translate(-circular_pixmap.width() / 2, -circular_pixmap.height() / 2);
    painter.drawPixmap(0, 0, circular_pixmap);

    label_music_icon->setPixmap(rotated_pixmap);
}

QPixmap AudioDisplayWidget::createCircularPixmap(const QPixmap &pixmap)
{
    QSize size = pixmap.size();
    QPixmap circularPixmap(size);
    circularPixmap.fill(Qt::transparent);

    QPainter painter(&circularPixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.addEllipse(0, 0, size.width(), size.height());

    painter.setClipPath(path);
    painter.drawPixmap(0, 0, pixmap);

    return circularPixmap;
}

// void AudioDisplayWidget::updatePCM(const QImage &image)
// {
//     // label_front->setPixmap(QPixmap::fromImage(image).scaled(label_front->size(), Qt::KeepAspectRatio));
// }