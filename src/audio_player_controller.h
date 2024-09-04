#ifndef __AUDIOPLAYERCONTROLLER_H__
#define __AUDIOPLAYERCONTROLLER_H__

#include <QAudioOutput>
#include <QIODevice>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <fstream>

extern "C" 
{
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
}

class AudioPlayerController : public QThread 
{
    Q_OBJECT

public:
    explicit AudioPlayerController(QObject *parent = nullptr);
    ~AudioPlayerController();

    Q_INVOKABLE void play();
    Q_INVOKABLE void stop();

    bool decode(uint8_t* data, size_t size);

signals:
    void pcmReady(const QImage &image); 
    void errorOccurred(const QString &errorString);

public slots:
    void GetMp3FileName(const QString &filename);

protected:
    void run() override;

private:
    void parseID3Tags(); 

private:
    bool                         playing_;
    bool                         open_mp3_;
    QMutex                       mutex_;
    QAudioOutput*                audio_output_;
    QIODevice*                   audio_device_;
    QString                      current_music_name_;
    int                          bytes_per_sample_;
    int                          sample_rate_;
    int                          channels_;
    size_t                       counter_;
    AVCodec*                     codec_;
    AVCodecContext*              codec_context_;
    AVFrame*                     frame_;
    AVPacket*                    pkt_;
    SwrContext*                  swr_ctx_;
    std::shared_ptr<uint8_t>     adts_header_;
    std::shared_ptr<std::thread> decode_thread_;
    std::ifstream                mp3_file_;
};

#endif  // __AUDIOPLAYERCONTROLLER_H__