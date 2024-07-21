#ifndef _ASR_LLM_TTS_H
#define _ASR_LLM_TTS_H

#include <iostream>
#include <string>
#include <curl/curl.h>
#include </usr/include/jsoncpp/json/json.h>
#include <portaudio.h>
#include <sndfile.h>
#include <fstream>  
#include <ctime>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

std::string getBaiduASRToken();

std::string speechToText(const std::string& asrToken, const std::string& audioFile);

std::string getLLMResponse(const std::string& inputText);

bool textToSpeech(const std::string& text, const std::string& outputFile);

std::string asr();

#endif