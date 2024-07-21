#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <portaudio.h>
#include <webrtc_vad.h>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>

#define SAMPLE_RATE 16000
#define FRAMES_PER_BUFFER 512
#define NUM_CHANNELS 1
#define SAMPLE_FORMAT paInt16
#define AUDIO_FILE "output.wav"

// 百度ASR配置
const std::string ASR_APP_ID = "87144823";
const std::string ASR_API_KEY = "lL2cjPgr40CBqr6MwyjkMOEi";
const std::string ASR_SECRET_KEY = "JB4DY5Ki7HVS3loGFnNHDu72497cr14c";

// 回调函数，用于处理CURL的响应数据
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string getBaiduASRToken() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://aip.baidubce.com/oauth/2.0/token?client_id=" + ASR_API_KEY + "&client_secret=" + ASR_SECRET_KEY + "&grant_type=client_credentials";
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        Json::Value jsonData;
        Json::Reader jsonReader;
        if (jsonReader.parse(readBuffer, jsonData)) {
            return jsonData["access_token"].asString();
        }
    }
    return "";
}

std::string speechToText(const std::string& asrToken, const std::string& audioFile) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    // Read audio file
    FILE* fp = fopen(audioFile.c_str(), "rb");
    if (!fp) {
        std::cerr << "Unable to open audio file: " << audioFile << std::endl;
        return "";
    }
    fseek(fp, 0, SEEK_END);
    long audioFileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* audioData = new char[audioFileSize];
    fread(audioData, 1, audioFileSize, fp);
    fclose(fp);

    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://vop.baidu.com/server_api?dev_pid=1537&cuid=" + ASR_APP_ID + "&token=" + asrToken;
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: audio/pcm; rate=16000");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, audioData);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, audioFileSize);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        delete[] audioData;

        Json::Value jsonData;
        Json::Reader jsonReader;
        if (jsonReader.parse(readBuffer, jsonData)) {
            return jsonData["result"][0].asString();
        }
    }
    return "";
}

typedef struct {
    int16_t *buffer;
    size_t max_size;
    size_t current_size;
} RecordBuffer;

static int RecordCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData) {
    RecordBuffer *data = (RecordBuffer *) userData;
    const int16_t *in = (const int16_t *) inputBuffer;

    if (data->current_size + framesPerBuffer < data->max_size) {
        memcpy(data->buffer + data->current_size, in, framesPerBuffer * sizeof(int16_t));
        data->current_size += framesPerBuffer;
    }
    return paContinue;
}

int main(void) {
    // 初始化 PortAudio
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "Failed to initialize PortAudio: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // 录音缓冲区
    RecordBuffer record_buffer;
    record_buffer.max_size = SAMPLE_RATE * 10; // 最长录音时间 10 秒
    record_buffer.buffer = (int16_t *) malloc(record_buffer.max_size * sizeof(int16_t));
    record_buffer.current_size = 0;

    // 打开音频流
    PaStream *stream;
    err = Pa_OpenDefaultStream(&stream, NUM_CHANNELS, 0, SAMPLE_FORMAT,
                               SAMPLE_RATE, FRAMES_PER_BUFFER, RecordCallback, &record_buffer);
    if (err != paNoError) {
        std::cerr << "Failed to open audio stream: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // 开始录音
    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start audio stream: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // 语音活动检测
    VadInst *vad = WebRtcVad_Create();
    WebRtcVad_Init(vad);
    WebRtcVad_set_mode(vad, 2); // 设置 VAD 模式

    std::cout << "Recording... Speak now!" << std::endl;
    int16_t frame[FRAMES_PER_BUFFER];
    int is_speech = 0;

    // 记录语音数据
    while (1) {
        if (Pa_ReadStream(stream, frame, FRAMES_PER_BUFFER) != paNoError) {
            std::cerr << "Failed to read audio stream" << std::endl;
            break;
        }

        if (WebRtcVad_Process(vad, SAMPLE_RATE, frame, FRAMES_PER_BUFFER) == 1) {
            // 用户在说话
            memcpy(record_buffer.buffer + record_buffer.current_size, frame, FRAMES_PER_BUFFER * sizeof(int16_t));
            record_buffer.current_size += FRAMES_PER_BUFFER;
        } else {
            // 用户停止说话
            std::cout << "Stopped recording." << std::endl;
            break;
        }
    }

    // 停止流
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
    Pa_Terminate();

    // 保存录音
    FILE *wav_file = fopen(AUDIO_FILE, "wb");
    if (wav_file) {
        // 写入 WAV 文件头
        // 假设每个样本是 16 位 (2 bytes)
        int32_t subchunk2_size = record_buffer.current_size * 2;
        int32_t chunk_size = 36 + subchunk2_size;
        fwrite("RIFF", 1, 4, wav_file);
        fwrite(&chunk_size, 4, 1, wav_file);
        fwrite("WAVE", 1, 4, wav_file);
        fwrite("fmt ", 1, 4, wav_file);

        int32_t subchunk1_size = 16;
        int16_t audio_format = 1;
        int16_t num_channels = NUM_CHANNELS;
        int32_t sample_rate = SAMPLE_RATE;
        int32_t byte_rate = SAMPLE_RATE * NUM_CHANNELS * 2;
        int16_t block_align = NUM_CHANNELS * 2;
        int16_t bits_per_sample = 16;

        fwrite(&subchunk1_size, 4, 1, wav_file);
        fwrite(&audio_format, 2, 1, wav_file);
        fwrite(&num_channels, 2, 1, wav_file);
        fwrite(&sample_rate, 4, 1, wav_file);
        fwrite(&byte_rate, 4, 1, wav_file);
        fwrite(&block_align, 2, 1, wav_file);
        fwrite(&bits_per_sample, 2, 1, wav_file);

        fwrite("data", 1, 4, wav_file);
        fwrite(&subchunk2_size, 4, 1, wav_file);
        fwrite(record_buffer.buffer, sizeof(int16_t), record_buffer.current_size, wav_file);
        fclose(wav_file);
    } else {
        std::cerr << "Unable to open output.wav for writing" << std::endl;
    }

    // 获取百度ASR Token
    std::string asrToken = getBaiduASRToken();
    if (asrToken.empty()) {
        std::cerr << "Failed to get ASR token" << std::endl;
        return 1;
    }

    // 进行语音识别
    std::string result = speechToText(asrToken, AUDIO_FILE);
    std::cout << "Recognized Text: " << result << std::endl;

    // 释放资源
    free(record_buffer.buffer);
    WebRtcVad_Free(vad);

    return 0;
}
