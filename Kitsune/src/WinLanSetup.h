#pragma once

#include <string>
#include <vector>

namespace Kitsune
{
    namespace WinLanSetup
    {
        struct EthernetAdapter
        {
            std::wstring NameW;  
            std::string  Name;   
        };

        std::vector<EthernetAdapter> GetEthernetAdapters();

        bool EnableLanMethod(const std::wstring& adapterName);

        bool DisableLanMethod(const std::wstring& adapterName);
    }
}