#define _CRT_SECURE_NO_WARNINGS
#include "Interface.h"
#include "imgui.h"
#include <thread>
#include <iostream>
#include <vector> 
#include <string>
#include "FTPManager.h"
#include "GoldHEN.h"
#include "FileDialog.h"
#include "WinLanSetup.h"


namespace Kitsune {
    namespace Interface {

        // Default Settings
        struct AppState {
            bool IsConnect = false; //Tener por default en false, solo true para Testeo
            char consoleIP[64] = "192.168.18.17"; //127.0.0.1
            char PcIP[64] = "127.0.0.1";
            int port = 2121;
            char RemotePath[64] = "/data/GoldHEN";
            bool Debug = false;
            char consoleLanIP[64] = "10.0.0.2";

            std::vector<std::string> logs;
            bool scrollToBottom = false; 
        };
        AppState state;

        // Declaración de funciones
        void InitMenu();
        void MainMenu();
        void Output();
        void ModManager();
        void PkgSender();

        void AddLog(const std::string& mensaje) {
            state.logs.push_back(mensaje);
            state.scrollToBottom = true; 

            std::cout << mensaje << std::endl;
        }

        void Render() {
            if (state.IsConnect == false)
            {
                InitMenu();
            }
            else
            {
                MainMenu();
            }
            Output();
        }

        // InitMenu
        void InitMenu() {
            ImGui::Begin("Kitsune Connection Setup");
            ImGui::Text("Welcome to Kitsune!\nA place where you have control over your console.");
            ImGui::Spacing();

            ImGui::Text("IP:");
            ImGui::InputText("##IPField", state.consoleIP, IM_ARRAYSIZE(state.consoleIP));

            ImGui::Text("Port:");
            ImGui::InputInt("##PortField", &state.port);
            ImGui::Spacing();

            if (ImGui::Button("Connect") && state.port > 0) {
                AddLog("[UI] Attempting to connect...");
                std::string Connection = Kitsune::FTP::FTPConnect(state.consoleIP, state.port);
                if (Connection.empty()) {
                    state.IsConnect = true;
                    AddLog("[UI] Successfully connected!");
                }
                else {
                    state.IsConnect = false;
                    AddLog("[UI] Connection failed.");
                }
            }

            ImGui::End();
        }

        // Main menu es el menu principal
        struct InterfaceMenu
        {
            bool showModManager = false; // Variable para controlar la visibilidad del Mod Manager
            bool showPkgSender = false; // Variable para controlar la visibilidad del Pkg Sender
        };
        InterfaceMenu _Menu;

        void MainMenu() {
            ImGui::Begin("Kitsune Control Panel - Connected");
            ImGui::Text("Welcome to the Kitsune Control Panel!");
            ImGui::Text("You are successfully connected to the console.");
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::Text("Available Options:");
            ImGui::Text("FileManager");

            ImGui::InputText("##Path", state.RemotePath, IM_ARRAYSIZE(state.RemotePath));
            ImGui::SameLine();
            if (ImGui::Button("ListDir")) {
                std::string resultado = Kitsune::FTP::FTPListDirectory(state.consoleIP, state.port, state.RemotePath);
                AddLog("[FTP] Listing directory:\n" + resultado);
            }

            ImGui::Text("Options");
            ImGui::Spacing();
            ImGui::Separator();

            //Butones de los menus
			if (ImGui::Button("Mods/Plugins Manager")) _Menu.showModManager = !_Menu.showModManager;
            ImGui::Spacing();
            if (ImGui::Button("Pkg Sender")) _Menu.showPkgSender = !_Menu.showPkgSender;

			if (_Menu.showModManager) ModManager();
            if (_Menu.showPkgSender) PkgSender();

            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Disconnect")) {
                AddLog("[UI] Disconnecting...");
                state.IsConnect = false;
            }
            ImGui::End();
        }
        //Pkg Sender
		bool isSendingPkg = false; // Variable para controlar el estado de envío de PKG
		bool isUseLan = false; // Variable para controlar si se usa LAN o no
		bool isUseCustomPort = false; // Variable para controlar si se usa un puerto personalizado
		int RpiPort = 12800; // Default port for RPI
        char pkgPath[4096] = "C:\\game.pkg"; 
        static std::vector<Kitsune::WinLanSetup::EthernetAdapter> adapters;
        static int selectedAdapter = 0;
        static bool initialized = false;

        void PkgSender() {
            ImGui::Begin("Kitsune Pkg Sender");
            ImGui::Text("Welcome to the Kitsune Pkg Sender!");
            ImGui::Text("Here you can send PKG files to your console.");
            ImGui::BulletText("Use LAN: Faster transfers. Requires a LAN cable.");
            ImGui::BulletText("Custom Port: Enable if your RPI uses a port other than 12800.");
            ImGui::Spacing();
            ImGui::Separator();

            if (!initialized)
            {
                adapters = Kitsune::WinLanSetup::GetEthernetAdapters();
                initialized = true;
            }
            if (ImGui::Checkbox("Use LAN", &isUseLan))
            {
                if (isUseLan)
                {
                    AddLog("[UI] LAN mode enabled.");
                }
                else
                {
                    AddLog("[UI] Restoring DHCP configuration...");
                    if (!adapters.empty() && selectedAdapter < (int)adapters.size())
                    {
                        if (Kitsune::WinLanSetup::DisableLanMethod(adapters[selectedAdapter].NameW))
                        {
                            AddLog("[UI] LAN method disabled.");
                        }
                    }
                }
            }

            if (isUseLan)
            {
                if (adapters.empty())
                {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1),"No Ethernet adapters found.");
                    if (ImGui::Button("Refresh"))
                    {
                        adapters = Kitsune::WinLanSetup::GetEthernetAdapters();
                    }
                }
                else
                {
                    if (selectedAdapter >= (int)adapters.size()) selectedAdapter = 0;

                    if (ImGui::BeginCombo("Ethernet Adapter",adapters[selectedAdapter].Name.c_str()))
                    {
                        for (int i = 0; i < adapters.size(); i++)
                        {
                            bool selected = (selectedAdapter == i);

                            if (ImGui::Selectable(
                                adapters[i].Name.c_str(),
                                selected))
                            {
                                selectedAdapter = i;
                            }

                            if (selected)
                                ImGui::SetItemDefaultFocus();
                        }

                        ImGui::EndCombo();
                    }

                    if (ImGui::Button("Refresh"))
                    {
                        adapters = Kitsune::WinLanSetup::GetEthernetAdapters();

                        if (selectedAdapter >= (int)adapters.size()) selectedAdapter = 0;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Configure LAN"))
                    {
                        if (Kitsune::WinLanSetup::EnableLanMethod(adapters[selectedAdapter].NameW))
                        {
                            AddLog("[UI] LAN configured successfully.");
                        }
                        else
                        {
                            AddLog("[UI] Failed to configure LAN.");
                        }
                    }
                }
                ImGui::Spacing();
                ImGui::Text("Console LAN IP:");
                ImGui::InputText("##ConsoleLanIP", state.consoleLanIP,IM_ARRAYSIZE(state.consoleLanIP));

                ImGui::Spacing();
            }

            if (ImGui::Checkbox("Use Custom Port", &isUseCustomPort)) {
                if (isUseCustomPort) AddLog("[UI] Using custom port for PKG sending.");
                else AddLog("[UI] Using default port (12800) for PKG sending.");
            }

			if (isUseCustomPort) {
				ImGui::Text("RPI Port:");
				ImGui::InputInt("##RpiPort", &RpiPort);
                ImGui::Spacing();
			}
            

            ImGui::Spacing();
            ImGui::Text("PC IP:");
            ImGui::InputText("##LocalIP", state.PcIP, IM_ARRAYSIZE(state.PcIP)); ImGui::SameLine();
            if (ImGui::Button("Detect Local IP")) {
                std::string localIP = Kitsune::FTP::GetLocalIP();
                if (!localIP.empty()) 
                {
                    strncpy(state.PcIP, localIP.c_str(), sizeof(state.PcIP) - 1);
                    state.PcIP[sizeof(state.PcIP) - 1] = '\0';
                }
            }

            ImGui::InputText("##PkgPath", pkgPath, IM_ARRAYSIZE(pkgPath)); ImGui::SameLine();
;

            if (ImGui::Button("Browse PKG")) {
                std::string selectedPath;
                if (OpenFileDialog(selectedPath, "Select PKG File", "*.pkg")) {
                    strncpy(pkgPath, selectedPath.c_str(), sizeof(pkgPath) - 1);
                    pkgPath[sizeof(pkgPath) - 1] = '\0';
                    AddLog("[UI] Selected PKG: " + selectedPath);
                }
            }

            if (isSendingPkg) {
                ImGui::BeginDisabled();
                ImGui::Button("Sending PKG... Please wait");
                ImGui::EndDisabled();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Server running. Check your PS4/PS5 Notifications!");
            }
            else 
            {
                ImGui::Spacing();
                if (ImGui::Button("Send PKG to PS4/PS5")) {
                    AddLog("[UI] Initializing transfer sequence...");
                    isSendingPkg = true;
                    std::string localPkgPath = std::string(pkgPath);
                    std::string localPcIP = std::string(state.PcIP);
					std::cout << "[UI] Local PC IP: " << localPcIP << std::endl;
                    std::string localConsoleIP = isUseLan ? std::string(state.consoleLanIP) : std::string(state.consoleIP);
                    int localRpiPort = RpiPort;

                    std::thread workerThread([localPkgPath, localPcIP, localConsoleIP, localRpiPort]()
                        {
                            std::thread ThreadServer(Kitsune::GoldHEN::StartLocalWebServer, localPkgPath);
                            ThreadServer.detach();

                            std::this_thread::sleep_for(std::chrono::seconds(1));

                            if (Kitsune::GoldHEN::SendRPICommand(localConsoleIP, localPcIP, localRpiPort, localPkgPath)) {
                                AddLog("[UI] PKG command accepted by PS4 successfully!");
                                isSendingPkg = false;
                            }
                            else {
                                AddLog("[UI] Failed to send PKG. Check console error codes.");
                                isSendingPkg = false;
                            }
                        });

                    workerThread.detach();
                }
            }
            ImGui::End();
        }

        //ModManager
		void ModManager() {
			ImGui::Begin("Kitsune Mod Manager");
			ImGui::Text("Welcome to the Kitsune Mod Manager!");
			ImGui::Text("Here you can manage your mods/plugins.");
			ImGui::Spacing();
			ImGui::Separator();
			//Mod Manager
			ImGui::End();
		}
        // OutPut
        void Output() {
            ImGui::Begin("Kitsune - Output");
            ImGui::Text("Logs:");
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::BeginChild("LogScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            for (const auto& log : state.logs) {
                ImGui::TextUnformatted(log.c_str());
                ImGui::Separator();
            }
            
            if (state.scrollToBottom) {
                ImGui::SetScrollHereY(1.0f);
                state.scrollToBottom = false;
            }

            ImGui::EndChild();
            ImGui::End();
        }

    } 
}