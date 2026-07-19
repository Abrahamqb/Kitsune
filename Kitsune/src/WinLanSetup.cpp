#define _WIN32_WINNT 0x0600

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <vector>
#include <windows.h>
#include <string>
#include "WinLanSetup.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

namespace Kitsune {
	namespace WinLanSetup {
        std::string WStringToString(const std::wstring& wstr)
        {
            if (wstr.empty()) return "";
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
            std::string strTo(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
            return strTo;
        }

        std::vector<EthernetAdapter> GetEthernetAdapters()
        {
            std::vector<EthernetAdapter> result;
            ULONG size = 0;
            GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, nullptr, &size);

            IP_ADAPTER_ADDRESSES* adapters = (IP_ADAPTER_ADDRESSES*)malloc(size);

            if (!adapters)
                return result;

            if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapters, &size) != NO_ERROR)
            {
                free(adapters);
                return result;
            }

            for (IP_ADAPTER_ADDRESSES* current = adapters;
                current;
                current = current->Next)
            {
                if (current->IfType == IF_TYPE_ETHERNET_CSMACD)
                {
                    std::wstring nameW = current->FriendlyName;
                    std::string name = WStringToString(nameW);
                    result.push_back({ nameW, name });
                }
            }

            free(adapters);
            return result;
        }

        bool EnableLanMethod(const std::wstring& adapterName)
        {
            if (adapterName.empty())
                return false;

            std::wstring powershellCmd =
                L"powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \""
                L"$m = New-Object -ComObject HNetCfg.HNetShare; "
                L"$privName = '" + adapterName + L"'; "
                L"$privateConnection = $m.EnumEveryConnection | Where-Object { try { $m.NetConnectionProps.Invoke($_).Name -eq $privName } catch { $false } }; "
                L"$publicConnection = $m.EnumEveryConnection | Where-Object { try { $n = $m.NetConnectionProps.Invoke($_).Name; $n -ne $privName -and ($n -like '*Wi-Fi*' -or $n -like '*WiFi*' -or $n -like '*Wireless*' -or $n -like '*Inal*') } catch { $false } }; "
                L"if (-not $publicConnection) { $publicConnection = $m.EnumEveryConnection | Where-Object { try { $m.NetConnectionProps.Invoke($_).Name -ne $privName } catch { $false } } | Select-Object -First 1 }; "
                L"if ($publicConnection -and $privateConnection) { "
                L"  try { "
                L"    $pubConfig = $m.INetSharingConfigurationForINetConnection.Invoke($publicConnection); "
                L"    $privConfig = $m.INetSharingConfigurationForINetConnection.Invoke($privateConnection); "
                L"    $pubConfig.EnableSharing(0); "
                L"    $privConfig.EnableSharing(1); "
                L"  } catch {} "
                L"}\"";

            return (_wsystem(powershellCmd.c_str()) == 0);
        }

        bool DisableLanMethod(const std::wstring& adapterName)
        {
            if (adapterName.empty())
                return false;

            std::wstring powershellCmd =
                L"powershell -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command \""
                L"$m = New-Object -ComObject HNetCfg.HNetShare; "
                L"$m.EnumEveryConnection | ForEach-Object { "
                L"  try { "
                L"    $c = $m.INetSharingConfigurationForINetConnection.Invoke($_); "
                L"    if ($c -and $c.SharingEnabled) { $c.DisableSharing() } "
                L"  } catch {} "
                L"}\"";

            return (_wsystem(powershellCmd.c_str()) == 0);
        }
	}
}