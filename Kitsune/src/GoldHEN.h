#pragma once
#include <iostream>
#include <string>

namespace Kitsune
{
	namespace GoldHEN {
		void StartLocalWebServer(std::string PathPKG);
		bool SendRPICommand(std::string PsIp, std::string PCIP, int Port, std::string PKGName);
	}
}