#ifndef __AUDIODISPLAYWIDGET_H__
#define __AUDIODISPLAYWIDGET_H__

#include <QWidget>
#include <QLabel>
#include <QGridLayout>
#include <QImage>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFileDialog>
#include <QString>
#include <QPushButton>

class AudioDisplayWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioDisplayWidget(QWidget *parent = nullptr);

private:
    void rotateIcon();
    QPixmap createCircularPixmap(const QPixmap &pixmap);

public slots:
    // void updatePCM(const QImage &image);
    void onOpenMp3ButtonClicked();

signals:
    // void GetKeyStateChanged(const QSet<int> &keys);
    void GetMp3FileName(const QString &filename);

private:
    QLabel      *label_music_icon;
    QLabel      *label_music_info;
    QLabel      *label_music_author;
    QPushButton *button_open_mp3;
    QFileDialog *file_dialog;
    QPixmap     circular_pixmap;
    int         rotation_angle;
};

#endif // __AUDIODISPLAYWIDGET_H__