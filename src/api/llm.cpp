#include <iostream>
#include <string>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>
#include <portaudio.h>
#include <sndfile.h>
#include <fstream>  
#include <ctime>
#include <./include/asr_llm_tts.h>

const std::string DEEPSEEK_API_KEY = "sk-8afc1b894a2d4aa0910ef89607dda0f1";

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