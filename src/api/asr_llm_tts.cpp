#include <iostream>
#include <string>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>
#include <portaudio.h>
#include <sndfile.h>
#include <fstream>  
#include <ctime>
// 百度ASR配置
const std::string ASR_APP_ID = "87144823";
const std::string ASR_API_KEY = "lL2cjPgr40CBqr6MwyjkMOEi";
const std::string ASR_SECRET_KEY = "JB4DY5Ki7HVS3loGFnNHDu72497cr14c";
const std::string DEEPSEEK_API_KEY = "sk-8afc1b894a2d4aa0910ef89607dda0f1";
const std::string TTS_APP_ID = "87144823";
const std::string TTS_API_KEY = "lL2cjPgr40CBqr6MwyjkMOEi";
const std::string TTS_SECRET_KEY= "JB4DY5Ki7HVS3loGFnNHDu72497cr14c";

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

std::string getLLMResponse(const std::string& inputText) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    
    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://api.deepseek.com/v1/chat/completions";
        std::string payload = "{\"model\":\"deepseek-chat\",\"messages\":[{\"role\":\"system\",\"content\":\"你是一个18岁女孩，温柔，回答的时候问题的时候娇羞，每次回答的字数不会超过20个字\"},{\"role\":\"user\",\"content\":\"" + inputText + "\"}],\"stream\":false}";

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + DEEPSEEK_API_KEY).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        Json::Value jsonData;
        Json::Reader jsonReader;
        if (jsonReader.parse(readBuffer, jsonData)) {
            return jsonData["choices"][0]["message"]["content"].asString();
        }
    }
    return "";
}

bool textToSpeech(const std::string& text, const std::string& outputFile) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl = curl_easy_init();
    if(curl) {
        std::string url = "https://tsn.baidu.com/text2audio";
        std::string payload = "tex=" + text + "&tok=" + getBaiduASRToken() + "&cuid=" + ASR_APP_ID + "&ctp=1&lan=zh&spd=5";

        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        std::ofstream outFile(outputFile, std::ios::binary);
        if (!outFile) {
            std::cerr << "Error opening output file: " << outputFile << std::endl;
            return false;
        }
        outFile.write(readBuffer.c_str(), readBuffer.size());
        outFile.close();
        
        return true;
    }
    return false;
}
/*void time_now()
{
    
    std::time_t now = std::time(nullptr);
    
    // 转换为本地时间
    std::tm* localTime = std::localtime(&now);
    
    // 打印时间戳
    std::cout << "当前时间: " << std::asctime(localTime);
    
}*/

int main() {
    //time_now();
    std::string asrToken = getBaiduASRToken();
    std::string inputAudio = "input.wav";  // 输入音频文件路径
    std::string outputAudio = "output.wav";  // 输出音频文件路径
    //time_now();
    // 1. ASR: 语音转文本
    std::string text = speechToText(asrToken, inputAudio);
    std::cout << "识别到的文本: " << text << std::endl;
    //time_now();
    // 2. LLM: 生成回复文本
    std::string responseText = getLLMResponse(text);
    std::cout << "LLM生成的回复: " << responseText << std::endl;
    //time_now();
    // 3. TTS: 文本转语音
    if (textToSpeech(responseText, outputAudio)) {
        std::cout << "合成的语音已保存到: " << outputAudio << std::endl;
    }

  
    return 0;
}
