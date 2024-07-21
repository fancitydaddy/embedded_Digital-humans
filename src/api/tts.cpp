#include <iostream>
#include <string>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>
#include <portaudio.h>
#include <sndfile.h>
#include <fstream>  
#include <ctime>
#include <./include/asr_llm_tts.h>

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