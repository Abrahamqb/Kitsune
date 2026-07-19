#include <iostream>
#include <string>
#include "imgui.h"
#include <curl/curl.h>

namespace Kitsune {
	namespace FTP {

		//callback
		size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
			size_t totalSize = size * nmemb;
			userp->append((char*)contents, totalSize);
			return totalSize;
		}

		std::string FTPConnect(std::string IP, int Port) {
			std::cout << "try: " << IP << " Port: " << Port << std::endl;
			CURL* curl = curl_easy_init();
			if (!curl) {
				return "Error: LibCurl.";
			}

			//FTP Setup
			std::string ftp = "ftp://" + IP + ":" + std::to_string(Port) + "/";
			curl_easy_setopt(curl, CURLOPT_URL, ftp.c_str());

			//Timeout 5 seconds
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

			//Pasive mode (MOISES)
			curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);

			//No se ingles, verififca la conexion y ya
			curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

			//Mute Flags
			curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

			//Intenta las conexion
			CURLcode res = curl_easy_perform(curl);
			//Liberamos memoria
			curl_easy_cleanup(curl);

			if (res == CURLE_OK)
			{
				return "";
			}
			else {
				return "Error";
			}

		}

		std::string FTPListDirectory(std::string IP, int Port, std::string RemotePath) {
			CURL* curl = curl_easy_init();
			std::string responseString = "";
			if (!curl) {
				return "Error: LibCurl.";
			}

			if (RemotePath.empty()) RemotePath = "/";
			if (RemotePath.back() != '/') RemotePath += "/";

			std::string ftp = "ftp://" + IP + ":" + std::to_string(Port) + RemotePath;
			curl_easy_setopt(curl, CURLOPT_URL, ftp.c_str());

			//configura el callback para capturar el texto de la ps
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);

			//Timeout 5
			curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);

			//Pasive mode (MOISES)
			curl_easy_setopt(curl, CURLOPT_FTP_SKIP_PASV_IP, 1L);

			CURLcode res = curl_easy_perform(curl);
			curl_easy_cleanup(curl);

			if (res == CURLE_OK) {
				return responseString;
			}
			else {
				return "Error listing directory: " + std::string(curl_easy_strerror(res));
			}

		}

		std::string GetLocalIP() {
			CURL* curl;
			CURLcode res;
			std::string result = "Error: Unknown error";

			curl = curl_easy_init();
			if (curl) {
				curl_easy_setopt(curl, CURLOPT_URL, "https://google.com");
				curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

				curl_easy_setopt(curl, CURLOPT_INTERFACE, "0.0.0.0");

				res = curl_easy_perform(curl);

				if (res == CURLE_OK) {
					char* localIP = nullptr;
					curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &localIP);
					if (localIP) {
						result = std::string(localIP);
					}
					else {
						result = "Error: IP not extracted";
					}
				}
				else {
					result = "Network error: " + std::string(curl_easy_strerror(res));
				}
				curl_easy_cleanup(curl);
			}
			else {
				result = "Error: Failed to initialize CURL";
			}

			return result;
		}

	}
}