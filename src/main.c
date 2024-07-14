#include <iostream>
#include <fstream>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>

std::string apiKey = "YOUR_OPENAI_API_KEY"; // 你需要替换为你的实际API密钥

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void saveToFile(const std::string& data, const std::string& filename) {
    std::ofstream outFile(filename, std::ios::binary);
    outFile.write(data.c_str(), data.size());
    outFile.close();
}

int main() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("Authorization: Bearer " + apiKey).c_str());
        headers = curl_slist_append(headers, "Content-Type: application/json");

        Json::Value jsonData;
        jsonData["model"] = "tts-1";
        jsonData["voice"] = "alloy";
        jsonData["input"] = "Hello world! This is a streaming test.";
        Json::StreamWriterBuilder writer;
        std::string jsonString = Json::writeString(writer, jsonData);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.openai.com/v1/audio/speech");
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonString.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);

        if(res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        else {
            saveToFile(readBuffer, "output.mp3");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return 0;
}
