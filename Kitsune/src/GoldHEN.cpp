#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define CPPHTTPLIB_NO_EXCEPTIONS
#include <iostream>
#include <string>
#include <thread>
#include <curl/curl.h>
#include <filesystem>

#include "http/httplib.h" 

namespace Kitsune
{
    namespace GoldHEN {

		size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
		{
			((std::string*)userp)->append((char*)contents, size * nmemb);
			return size * nmemb;
		}
        
        void StartLocalWebServer(std::string PathPKG) {
            if (PathPKG.size() >= 2 && PathPKG.front() == '"' && PathPKG.back() == '"') {
                PathPKG = PathPKG.substr(1, PathPKG.size() - 2);
            }
            httplib::Server svr;

			svr.Get("/.*",
			[PathPKG](const httplib::Request& req, httplib::Response& res)
			{
				std::cout << "[Server] Received request for: " << req.path << std::endl;
				if (!std::filesystem::exists(PathPKG)) {
					std::cerr << "[Server Error] File does not exist: " << PathPKG << std::endl;
					res.status = 404;
					return;
				}
				res.set_file_content(PathPKG.c_str(), "application/octet-stream");
				std::cout << "[Server] Served file: " << PathPKG << std::endl;
			});

			std::cout << "Starting local web server on http://localhost:8080" << std::endl;
            svr.listen("0.0.0.0", 8080);
        }

        bool SendRPICommand(std::string PsIp, std::string PCIP, std::string PKGName) {
            if (PKGName.size() >= 2 && PKGName.front() == '"' && PKGName.back() == '"') {
                PKGName = PKGName.substr(1, PKGName.size() - 2);
            }
			CURL* curl = curl_easy_init();
			if (!curl) {
				std::cerr << "Failed to initialize CURL" << std::endl;
				return false;
			}
			std::string FileName = std::filesystem::path(PKGName).filename().string();

			std::string RPIUrl = "http://" + PsIp + ":12800/api/install";
			std::string UrlPC = "http://" + PCIP + ":8080/" + FileName;
            std::string JsonPayload = "{\"type\":\"direct\", \"packages\":[\"" + UrlPC + "\"]}";

			curl_easy_setopt(curl, CURLOPT_URL, RPIUrl.c_str());
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JsonPayload.c_str());

            struct curl_slist* headers = NULL;
			headers = curl_slist_append(headers, "Content-Type: application/json");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
			std::cout << JsonPayload << std::endl;
			CURLcode res = curl_easy_perform(curl);

			std::string response;

			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

			std::cout << response << std::endl;

			curl_slist_free_all(headers);
			curl_easy_cleanup(curl);

			return (res == CURLE_OK);
        }
    }
}
