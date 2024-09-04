#include <audio_player_controller.h>
#include <iostream>
#include <cstring>
#include <thread>

AudioPlayerController::AudioPlayerController(QObject *parent)
    : QThread(parent)
    , open_mp3_(false)
    , audio_output_(nullptr)
    , audio_device_(nullptr)
    , codec_(nullptr)
    , codec_context_(nullptr)
    , frame_(nullptr)
    , pkt_(nullptr)
{
    // 设置音频输出
    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleSize(32);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::Float);      // 这里的SignedInt是int16，如果要设置为float，需要设置为QAudioFormat::Float

    audio_output_ = new QAudioOutput(format, this);
    audio_device_ = audio_output_->start();
}

AudioPlayerController::~AudioPlayerController()
{
    delete audio_output_;
    delete audio_device_;
    audio_output_->stop();
    av_packet_free(&pkt_);
    av_frame_free(&frame_);
    avcodec_free_context(&codec_context_);
    stop();
}

void AudioPlayerController::play()
{
    QMutexLocker locker(&mutex_);
    playing_ = true;
    if (!isRunning()) 
    {
        start();
    }
}

void AudioPlayerController::stop()
{
    QMutexLocker locker(&mutex_);
    playing_ = false;
    wait();
}

void AudioPlayerController::run()
{
#if LIBAVCODEC_VERSION_MAJOR < 58
    avcodec_register_all();
#endif

    // 查找MP3解码器
    codec_ = avcodec_find_decoder(AV_CODEC_ID_MP3);
    if (!codec_) 
    {
        std::cerr << "Codec not found" << std::endl;
        return;
    }

    // 分配解码器上下文
    codec_context_ = avcodec_alloc_context3(codec_);
    if (!codec_context_) 
    {
        std::cerr << "Could not allocate audio codec context" << std::endl;
        return;
    }

    if (avcodec_open2(codec_context_, codec_, nullptr) < 0) 
    {
        throw std::runtime_error("Could not open codec");
    }

    frame_ = av_frame_alloc();
    if (!frame_) 
    {
        throw std::runtime_error("Could not allocate audio frame");
    }

    pkt_ = av_packet_alloc();
    if (!pkt_) 
    {
        throw std::runtime_error("Could not allocate packet");
    }

    QThread::setPriority(QThread::HighPriority);

    std::vector<uint8_t> buffer(4096);

    while (playing_) 
    {
        if (!open_mp3_)
        {
            mp3_file_ = std::ifstream(current_music_name_.toStdString(), std::ios::binary);
            if (!mp3_file_) 
            {
                continue;
            }
        }

        open_mp3_ = true;

        // parseID3Tags();

        // 读取MP3帧头部（前4个字节用于检测帧同步字）
        mp3_file_.read(reinterpret_cast<char*>(buffer.data()), 4);
        if (mp3_file_.gcount() < 4) 
        {
            continue;
        }
        // 检查帧同步字 (11个连续的1)
        if ((buffer[0] == 0xFF) && ((buffer[1] & 0xE0) == 0xE0)) 
        {
            // 计算帧比特率
            int bitrate_index = (buffer[2] >> 4) & 0x0F;
            int bitrate = 0;
            static const int bitrates[] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0};
            if (bitrate_index > 0 && bitrate_index < 15) 
            {
                bitrate = bitrates[bitrate_index] * 1000; // 比特率的单位是kbps
            } 
            else 
            {
                std::cerr << "Invalid bitrate index: " << bitrate_index << std::endl;
                continue;
            }

            // 计算采样率
            int sample_rate_index = (buffer[2] >> 2) & 0x03;
            int sample_rate = 0;
            static const int sample_rates[] = {44100, 48000, 32000, 0};
            if (sample_rate_index < 3) 
            {
                sample_rate = sample_rates[sample_rate_index];
            } 
            else 
            {
                std::cerr << "Invalid sample rate index: " << sample_rate_index << std::endl;
                continue;
            }

            // 计算帧长度
            int padding = (buffer[2] >> 1) & 0x01;
            int frameLength = (144 * bitrate / sample_rate) + padding;

            // 检查帧长度是否有效
            if (frameLength < 7 || frameLength > 1441) 
            {
                std::cerr << "Invalid frame length: " << frameLength << std::endl;
                continue;
            }

            // 读取完整的MP3帧
            buffer.resize(frameLength);
            mp3_file_.read(reinterpret_cast<char*>(buffer.data() + 4), frameLength - 4);
            if (mp3_file_.gcount() < frameLength - 4) 
            {
                continue;
            }

            // 送入解码器
            if (!decode(buffer.data(), frameLength)) 
            {
                std::cerr << "Failed to decode MP3 frame" << std::endl;
                continue;
            }
        } 
        else 
        {
            // 如果未找到帧同步字，则跳过一个字节继续查找
            mp3_file_.seekg(-3, std::ios::cur);
        }
    }
}

bool AudioPlayerController::decode(uint8_t* data, size_t size)
{
    pkt_->data = data;
    pkt_->size = size;

    if (avcodec_send_packet(codec_context_, pkt_) < 0) 
    {
        std::cerr << "Error sending the packet to the decoder" << std::endl;
        return false;
        // return;
    }

    while (avcodec_receive_frame(codec_context_, frame_) == 0) 
    {
        {
            int buffer_size = av_samples_get_buffer_size(nullptr, codec_context_->channels, frame_->nb_samples, codec_context_->sample_fmt, 1);

            // 如果是平面格式（如fltp），需要分别处理每个声道的数据
            std::vector<uint8_t> interleaved_buffer(buffer_size);

            // 将各声道的数据交织在一起
            for (int sample = 0; sample < frame_->nb_samples; ++sample) 
            {
                for (int ch = 0; ch < codec_context_->channels; ++ch) 
                {
                    float* src = reinterpret_cast<float*>(frame_->data[ch]);
                    float* dst = reinterpret_cast<float*>(interleaved_buffer.data());
                    dst[sample * codec_context_->channels + ch] = src[sample];
                }
            }

            // 将交织后的数据传递给回调函数
            double duration_per_buffer = (buffer_size / sizeof(float)) / 44100.0 / 2;  // 计算每次写入的时长
            auto sleep_duration = std::chrono::milliseconds(static_cast<int>(duration_per_buffer * 1000));
            qint64 written = audio_device_->write(reinterpret_cast<const char*>(interleaved_buffer.data()), buffer_size);
            std::this_thread::sleep_for(sleep_duration);
        }
    }

    av_packet_unref(pkt_);
    return true;
}

void AudioPlayerController::GetMp3FileName(const QString &filename)
{
    current_music_name_ = filename;
    open_mp3_ = false;
}

void AudioPlayerController::parseID3Tags() 
{
    std::vector<uint8_t> buffer;
    char header[10];
    mp3_file_.read(header, 10);
    if (mp3_file_.gcount() < 10 || std::string(header, 3) != "ID3") 
    {
        // 没有ID3标签或ID3标签格式错误
        std::cerr << "No ID3v2 tag found." << std::endl;
        mp3_file_.seekg(0); // 回到文件开始，准备读取MP3帧
        return;
    }

    int tagSize = ((header[6] & 0x7F) << 21) | ((header[7] & 0x7F) << 14) |
                  ((header[8] & 0x7F) << 7) | (header[9] & 0x7F);

    buffer.resize(tagSize);
    mp3_file_.read(reinterpret_cast<char*>(buffer.data()), tagSize);

    size_t pos = 0;
    while (pos + 10 <= buffer.size()) 
    {
        std::string frameId(reinterpret_cast<char*>(&buffer[pos]), 4);
        int frameSize = (buffer[pos + 4] << 24) | (buffer[pos + 5] << 16) |
                        (buffer[pos + 6] << 8) | buffer[pos + 7];

        if (frameSize <= 0 || pos + 10 + frameSize > buffer.size()) 
        {
            break;
        }

        std::string frameData(reinterpret_cast<char*>(&buffer[pos + 10]), frameSize);
        if (frameId == "TIT2") 
        {
            std::cout << "Title: " << frameData.substr(1) << std::endl;
        } 
        else if (frameId == "TPE1") 
        {
            std::cout << "Artist: " << frameData.substr(1) << std::endl;
        } 
        else if (frameId == "TALB") 
        {
            std::cout << "Album: " << frameData.substr(1) << std::endl;
        }

        pos += 10 + frameSize;
    }
}