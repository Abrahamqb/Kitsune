#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define CPPHTTPLIB_NO_EXCEPTIONS
#include <iostream>
#include <string>
#include <thread>
#include <memory> 
#include <curl/curl.h>
#include <filesystem>

#include "http/httplib.h" 

namespace Kitsune
{
    namespace GoldHEN {
        std::unique_ptr<httplib::Server> svr = nullptr;
        bool isServerRunning = false;
        std::string currentActivePkgPath = "";

        size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
        {
            ((std::string*)userp)->append((char*)contents, size * nmemb);
            return size * nmemb;
        }

        void StopLocalWebServer() {
            if (isServerRunning && svr) {
                std::cout << "[Server] Stopping active web server instance..." << std::endl;
                svr->stop();
                isServerRunning = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(400));
            }
        }

        void StartLocalWebServer(std::string PathPKG) {
            if (PathPKG.size() >= 2 && PathPKG.front() == '"' && PathPKG.back() == '"') {
                PathPKG = PathPKG.substr(1, PathPKG.size() - 2);
            }

            if (PathPKG.length() > 4 && PathPKG.substr(PathPKG.length() - 4) != ".pkg") {
                size_t lastDot = PathPKG.find_last_of(".");
                if (lastDot != std::string::npos) {
                    PathPKG = PathPKG.substr(0, lastDot) + ".pkg";
                }
            }

            StopLocalWebServer();

            svr = std::make_unique<httplib::Server>();
            currentActivePkgPath = PathPKG;

            svr->Get("/.*", [](const httplib::Request& req, httplib::Response& res)
                {
                    std::cout << "[Server] Received request for: " << req.path << std::endl;
                    if (!std::filesystem::exists(currentActivePkgPath)) {
                        std::cerr << "[Server Error] File does not exist: " << currentActivePkgPath << std::endl;
                        res.status = 404;
                        return;
                    }
                    res.set_file_content(currentActivePkgPath.c_str(), "application/octet-stream");
                    std::cout << "[Server] Served file: " << currentActivePkgPath << std::endl;
                });

            std::cout << "Starting local web server on http://0.0.0.0" << std::endl;
            isServerRunning = true;
            svr->listen("0.0.0.0", 8080);
        }

        bool SendRPICommand(std::string PsIp, std::string PCIP, int Port, std::string PKGName) {
            if (PKGName.size() >= 2 && PKGName.front() == '"' && PKGName.back() == '"') {
                PKGName = PKGName.substr(1, PKGName.size() - 2);
            }

            if (PKGName.length() > 4 && PKGName.substr(PKGName.length() - 4) != ".pkg") {
                size_t lastDot = PKGName.find_last_of(".");
                if (lastDot != std::string::npos) {
                    PKGName = PKGName.substr(0, lastDot) + ".pkg";
                }
            }

            CURL* curl = curl_easy_init();
            if (!curl) {
                std::cerr << "Failed to initialize CURL" << std::endl;
                return false;
            }
            std::string FileName = std::filesystem::path(PKGName).filename().string();

            std::string RPIUrl = "http://" + PsIp + ":" + std::to_string(Port) + "/api/install";

            std::string UrlPC = "http:\\/\\/" + PCIP + ":8080\\/" + FileName;
            std::string JsonPayload = "{\"type\":\"direct\",\"packages\":[\"" + UrlPC + "\"]}";

            std::string response = "";

            curl_easy_setopt(curl, CURLOPT_URL, RPIUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JsonPayload.c_str());

            struct curl_slist* headers = NULL;
            headers = curl_slist_append(headers, "Content-Type: application/json");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            std::cout << "Payload: " << JsonPayload << std::endl;
            CURLcode res = curl_easy_perform(curl);

            std::cout << "PS4 Response: " << response << std::endl;

            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);

            if (res != CURLE_OK || response.find("\"status\":\"fail\"") != std::string::npos || response.find("\"status\": \"fail\"") != std::string::npos) {
                return false;
            }

            return true;
        }
    }
}
