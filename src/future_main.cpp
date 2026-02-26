#include "../external/imgui/imgui.h"
#include "../external/imgui/backends/imgui_impl_glfw.h"
#include "../external/imgui/backends/imgui_impl_opengl3.h"


#include "UI/sidebar_panel.h"
#include "UI/right_panel.h"
#include "future_app_state.h"



#include "storage.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <ctime>




int main() {
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(1200, 800, "MyBudget", NULL, NULL);
    if (!window) return 1;
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    // Single window: no viewports so the app and the OS window are one
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Query monitor DPI scale so the UI is readable on high-DPI laptops
    float xscale = 1.0f, yscale = 1.0f;
    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (primary)
        glfwGetMonitorContentScale(primary, &xscale, &yscale);
    float dpi_scale = (xscale > yscale) ? xscale : yscale;
    if (dpi_scale < 1.0f) dpi_scale = 1.0f;

    const float base_font_size  = 18.0f * dpi_scale;
    const float large_font_size = 24.0f * dpi_scale;

    ImFont* font = io.Fonts->AddFontFromFileTTF("../external/fonts/WorkSans-VariableFont_wght.ttf", base_font_size);
    if (font)
        io.FontDefault = font;
    ImFont* font_large = io.Fonts->AddFontFromFileTTF("../external/fonts/WorkSans-VariableFont_wght.ttf", large_font_size);
    if (!font_large)
        font_large = font;

    
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 8.f;
    style.ChildRounding = 6.f;
    style.FrameRounding = 6.f;
    style.PopupRounding = 4.f;
    style.ScrollbarRounding = 4.f;
    style.GrabRounding = 4.f;
    style.TabRounding = 4.f;
    style.FramePadding = ImVec2(12.f * dpi_scale, 8.f * dpi_scale);
    style.ItemSpacing = ImVec2(10.f * dpi_scale, 8.f * dpi_scale);
    style.ItemInnerSpacing = ImVec2(8.f * dpi_scale, 6.f * dpi_scale);
    style.WindowPadding = ImVec2(14.f * dpi_scale, 14.f * dpi_scale);

    
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]          = ImVec4(0.12f, 0.13f, 0.15f, 0.94f);
    colors[ImGuiCol_ChildBg]          = ImVec4(0.15f, 0.16f, 0.18f, 0.98f);
    colors[ImGuiCol_FrameBg]          = ImVec4(0.20f, 0.21f, 0.24f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]   = ImVec4(0.24f, 0.26f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive]    = ImVec4(0.27f, 0.29f, 0.32f, 1.00f);
    colors[ImGuiCol_Button]            = ImVec4(0.20f, 0.45f, 0.45f, 1.00f);  // teal
    colors[ImGuiCol_ButtonHovered]     = ImVec4(0.25f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_ButtonActive]      = ImVec4(0.30f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_Header]            = ImVec4(0.20f, 0.45f, 0.45f, 0.55f);
    colors[ImGuiCol_HeaderHovered]     = ImVec4(0.25f, 0.55f, 0.55f, 0.80f);
    colors[ImGuiCol_HeaderActive]      = ImVec4(0.30f, 0.65f, 0.65f, 1.00f);
    colors[ImGuiCol_Text]             = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
    colors[ImGuiCol_TextDisabled]      = ImVec4(0.50f, 0.52f, 0.55f, 1.00f);
    colors[ImGuiCol_Border]            = ImVec4(0.28f, 0.30f, 0.34f, 1.00f);
    colors[ImGuiCol_BorderShadow]      = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

    io.FontGlobalScale = 1.0f;

    //Open the main window
    bool open = true;
        
    //CREATE OUR DATABASE
    auto myDB = Storage();


    App_state state;
    state.dpi_scale = dpi_scale;
    Controller controller(state, myDB);
    state.wallet = myDB.load_accounts();
    myDB.load_all_transactions();




    while (!glfwWindowShouldClose(window)) 
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Fill the whole GLFW client area so window and app are one; use display size for correct click mapping
        const float win_w = io.DisplaySize.x;
        const float win_h = io.DisplaySize.y;
        ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
        ImGui::SetNextWindowSize(ImVec2(win_w, win_h));
        ImGui::Begin("My Budget App", &open,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);


        const float left_ratio = 0.25f;  // left pane = 1/4 of window
        const float left_pane_width = win_w * left_ratio - ImGui::GetStyle().ItemSpacing.x * 0.5f;
        const float right_pane_width = win_w * (1.f - left_ratio) - ImGui::GetStyle().ItemSpacing.x * 0.5f;

        Sidebar_result sidebar_result = draw_sidebar(state, controller, left_pane_width);
        if (sidebar_result.exit_requested)
        {
            glfwSetWindowShouldClose(window, true);
        }

        ImGui::SameLine();
        draw_right_panel(state, controller, right_pane_width, font_large);
        ImGui::End();

        ImGui::Render();
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}