#pragma once
#include <string>

namespace Kitsune {
	namespace FTP {
		std::string FTPConnect(std::string IP, int Port);

		std::string FTPListDirectory(std::string IP, int Port, std::string RemotePath);

		std::string GetLocalIP();
	}
}