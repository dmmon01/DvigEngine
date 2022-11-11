#include "../../include/DEGUI.hpp"

#include "../../thirdparty/glfw_win64/include/GLFW/glfw3.h"

void* DvigEngine2::IWindow::m_GLFWWindows[] = {};
DvigEngine2::IWindow* DvigEngine2::IWindow::m_WindowInstances[] = {};

void DvigEngine2::IWindow::Init()
{
    // Create GLFW window
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(640, 480, "DvigEngine Test", NULL, NULL);

    DV_ASSERT_PTR(window);

    // Assign to member variables
    this->m_GLFWWindow = (void*)window;

    // Add to global stack
    deint32 cycle = 0;
    while (this->m_GLFWWindows[cycle] != nullptr && ++cycle < DV_MAX_GUI_WINDOW_COUNT);
    DV_ASSERT( cycle < DV_MAX_GUI_WINDOW_COUNT );
    IWindow::m_GLFWWindows[cycle] = this->m_GLFWWindow;
    IWindow::m_WindowInstances[cycle] = this;
}

void DvigEngine2::IWindow::Free()
{
    DvigEngine2::Engine* engine = this->GetEngine();

    if (this->m_UserData != nullptr) {
        engine->Delete( this->m_UserData );    
    }

    engine->Delete( this->GetMemoryObject() );
}

void DvigEngine2::IWindow::Start()
{
    deint32 cycle = 0;
    deusize presentWindowCount = 0;
    do
    {
        // Count non-zero windows
        presentWindowCount = 0;
        for (deint32 i = 0; i < DV_MAX_GUI_WINDOW_COUNT; ++i) {
            if (IWindow::m_GLFWWindows[i] != nullptr) { presentWindowCount += 1; }
        }

        // Wrap cycle value
        if (cycle >= DV_MAX_GUI_WINDOW_COUNT) { cycle = 0; }

        // Get windows
        GLFWwindow** pGLFWWindow = (GLFWwindow**)&IWindow::m_GLFWWindows[cycle];
        IWindow* windowInstance = IWindow::m_WindowInstances[cycle];
        cycle += 1;

        // Get next window
        if (*pGLFWWindow == nullptr) { continue; }

        // Functionality
        if (!glfwWindowShouldClose(*pGLFWWindow))
        {
            windowInstance->Update();

            glfwSwapBuffers( *pGLFWWindow );
            glfwPollEvents();
        }
        else
        {
            // Destroy window
            glfwDestroyWindow( *pGLFWWindow );
            *pGLFWWindow = nullptr;
            windowInstance->Free();

            presentWindowCount -= 1;
        }
    } while (presentWindowCount > 0);
}