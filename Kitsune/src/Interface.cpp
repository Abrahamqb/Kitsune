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


namespace Kitsune {
    namespace Interface {

        // Default Settings
        struct AppState {
            bool IsConnect = false; //Tener por default en false, solo true para Testeo
            char consoleIP[64] = "192.168.18.17"; //127.0.0.1
            char PcIP[64] = "192.168.18.19";
            int port = 2121;
            char RemotePath[64] = "/data/GoldHEN";
            bool Debug = false;
            char consoleLanIP[64] = "127.0.0.2";

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
            Output();
            if (state.IsConnect == false)
            {
                InitMenu();
            }
            else
            {
                MainMenu();
            }
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
        char pkgPath[4096] = "C:\\game.pkg"; // Safe 4096 buffer size
        void PkgSender() {
            ImGui::Begin("Kitsune Pkg Sender");
            ImGui::Text("Welcome to the Kitsune Pkg Sender!");
            ImGui::Text("Here you can send PKG files to your console.");
            ImGui::Spacing();
            ImGui::Separator();

            if (ImGui::Checkbox("Use Lan", &isUseLan)) {
                if (isUseLan) AddLog("[UI] Using LAN for PKG sending.");
                else AddLog("[UI] Using default Wifi config for PKG sending.");
            }

            if (isUseLan) {
                ImGui::Text("Console Lan IP:");
                ImGui::InputText("##ConsoleLanIP", state.consoleLanIP, IM_ARRAYSIZE(state.consoleLanIP)); ImGui::SameLine();
                if (ImGui::Button("Set")) {
                    // Detect Console Lan IP
                }
            }

            ImGui::Spacing();
            ImGui::Text("PC IP:");
            ImGui::InputText("##LocalIP", state.PcIP, IM_ARRAYSIZE(state.PcIP)); ImGui::SameLine();
            if (ImGui::Button("Detect Local IP")) {
                // IP
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
                    std::string localConsoleIP = std::string(state.consoleIP);

                    std::thread workerThread([localPkgPath, localPcIP, localConsoleIP]()
                        {
                            std::thread ThreadServer(Kitsune::GoldHEN::StartLocalWebServer, localPkgPath);
                            ThreadServer.detach();

                            std::this_thread::sleep_for(std::chrono::seconds(1));

                            if (Kitsune::GoldHEN::SendRPICommand(localConsoleIP, localPcIP, localPkgPath)) {
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

    } // namespace Interface
} // namespace Kitsune