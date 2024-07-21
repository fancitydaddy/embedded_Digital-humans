#include <iostream>
#include <string>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>
#include <portaudio.h>
#include <sndfile.h>
#include <fstream>  
#include <ctime>
//#include <./include/asr_llm_tts.h>

const std::string ASR_APP_ID = "87144823";
const std::string ASR_API_KEY = "lL2cjPgr40CBqr6MwyjkMOEi";
const std::string ASR_SECRET_KEY = "JB4DY5Ki7HVS3loGFnNHDu72497cr14c";

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

std::string asr(){
    
    std::string asrToken = getBaiduASRToken();
    std::string inputAudio = "input.wav";  // 输入音频文件路径
    std::string text = speechToText(asrToken, inputAudio);

}