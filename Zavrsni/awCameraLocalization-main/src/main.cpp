#include <stdio.h>
#include <iostream>
#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <implot.h>
#include <imnodes.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include "SimpleClass.hpp"
#include "AwGui.hpp"

#include "NodeBase.hpp"
#include "NodeTest.hpp"
#include "NodeTest2.hpp"
#include "ConnectorBase.hpp"
#include "NodeInflate.hpp"
#include "ConnectorOut.hpp"
#include "ConnectorIn.hpp"
#include "ConnectionBase.hpp"
#include "Connection.hpp"
#include "ElementFactory.hpp"
#include "Message.hpp"
#include "Util.hpp"
#include "imgui_internal.h"

//#include <boost/container/vector.hpp>
//#include <boost/version.hpp>
//#include <boost/unordered_map.hpp>
//#include <boost/interprocess/ipc/message_queue.hpp>
//#include <boost/container/vector.hpp>
//#include <boost/thread.hpp>
#include <unordered_map>
#include <csignal>
//#include <boost/circular_buffer.hpp>

// Callback function for framebuffer size changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
void sigCallback(int signum){
    AwGui::getInstance().closeApp();
    exit(signum);
}
int main() {
    printf("Tu sam\n");
    std::signal(SIGINT, sigCallback);   // Handle Ctrl-C signal
    std::signal(SIGTERM, sigCallback);  // Handle termination signal


    const char* imguiVersion = ImGui::GetVersion();
    std::cout << "ImGui version: " << imguiVersion << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Minimal Example with OpenCV", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize Glad" << std::endl;
        glfwTerminate();
        return -1;
    }


    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImNodes::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Initialize OpenCV
    /*
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera" << std::endl;
        return -1;
    }
    */

    // Create OpenGL texture
    
    
    //InitializeAwGui
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        AwGui::getInstance().process();
        
        
        //ImGui::ShowDemoWindow();
        /*
        // Capture frame from camera
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) continue;

        // Convert frame to RGBA
        cv::Mat frameRGBA;
        cv::cvtColor(frame, frameRGBA, cv::COLOR_BGR2RGBA);

        // Upload frame to OpenGL texture
        Util::fillTexture(frameRGBA, textureID);

        

        // Example ImGui window
        ImGui::Begin("Camera Stream");
        ImVec2 imageSize(frame.cols, frame.rows);
        ImGui::Image((void*)(intptr_t)textureID, imageSize);
        ImGui::End();
        */
        // Render ImGui
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    //Window is closing...
    AwGui::getInstance().closeApp();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    //glDeleteTextures(1, &textureID);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
