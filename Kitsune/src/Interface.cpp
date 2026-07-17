#include "Interface.h"
#include "imgui.h"
#include <iostream>
#include <vector>  // <-- NUEVO: Para guardar la lista de mensajes
#include <string>  // <-- NUEVO: Para manejar los textos de los logs
#include "FTPManager.h"

namespace Kitsune {
    namespace Interface {

        // Default Settings
        struct AppState {
            bool IsConnect = false;
            char consoleIP[64] = "192.168.18.17";
            int port = 2121;
            char RemotePath[64] = "/data/GoldHEN";
            bool Debug = false;

            std::vector<std::string> logs;
            bool scrollToBottom = false; 
        };
        AppState state;

        // Declaración de funciones
        void InitMenu();
        void MainMenu();
        void Output();

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

            ImGui::Text("Settings (Coming soon)");
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Disconnect")) {
                AddLog("[UI] Disconnecting...");
                state.IsConnect = false;
            }
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