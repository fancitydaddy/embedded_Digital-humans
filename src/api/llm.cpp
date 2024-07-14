#include <iostream>
#include <string>
#include <curl/curl.h>

class GitAPI {
public:
    GitAPI(const std::string& api_key) : api_key(api_key) {
        // Initialize libcurl
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) {
            throw std::runtime_error("Failed to initialize libcurl");
        }
    }

    ~GitAPI() {
        // Cleanup libcurl resources
        curl_easy_cleanup(curl);
        curl_global_cleanup();
    }

    std::string request(const std::string& url, const std::string& post_fields) {
        std::string response; // Buffer to store the response
        struct curl_slist *headers = NULL;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set the HTTP headers
        headers = curl_slist_append(headers, "Content-Type: application/json");
        std::string auth_header = "Authorization: Bearer " + api_key;
        headers = curl_slist_append(headers, auth_header.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set the POST fields
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_fields.c_str());

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        // Clean up
        curl_slist_free_all(headers);

        return response;
    }

private:
    CURL *curl;
    std::string api_key;

    static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp) {
        userp->append((char *)contents, size * nmemb);
        return size * nmemb;
    }
};