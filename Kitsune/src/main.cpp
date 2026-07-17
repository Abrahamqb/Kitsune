#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// Cabeceras del proyecto
#include "Interface.h"

// Cabeceras de ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

//curl
#include <curl/curl.h>

int main() {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    // 1. Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Error initializing GLFW" << std::endl;
        return -1;
    }

    // Configure OpenGL version (3.3 Core)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create the window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Kitsune Manager - Dev Build", NULL, NULL);
    if (!window) {
        std::cerr << "Error creating GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable VSync (Stable 60 FPS)

    // 2. Initialize GLAD to load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error initializing GLAD" << std::endl;
        return -1;
    }

    // 3. Initialize ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\arial.ttf", 18.0f);

    // Default to fallback font if specific font file is not found
    if (font == nullptr) {
        std::cout << "Could not load font, using fallback font." << std::endl;
    }
    ImGui::StyleColorsDark(); // Dark style by default

    // Setup bindings for GLFW and OpenGL3
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Clear color for background (a very elegant dark blue/gray)
    float clear_color[4] = { 0.09f, 0.09f, 0.1f, 1.0f };

    // Application main loop
    while (!glfwWindowShouldClose(window)) {
        // Process system events (mouse, keyboard, window resize)
        glfwPollEvents();

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the modular user interface
        Kitsune::Interface::Render();

        // ImGui Rendering
        ImGui::Render();

        // Clear screen buffer using OpenGL
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui draw data to screen
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers to display new frame
        glfwSwapBuffers(window);
    }

    // Cleanup on close
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    curl_global_cleanup();
    return 0;
}