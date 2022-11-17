// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#pragma comment(lib, "d3d11.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "Stb/stb_image.h"
#include "Imgui/imgui.h"
#include "Imgui/imgui_impl_win32.h"
#include "Imgui/imgui_impl_dx11.h"
#include "Imgui/imgui-knobs.h"
#include "IconFont/IconsFontAwesome4.h"
#include <d3d11.h>
#include <tchar.h>
#include <thread>
#include <stdio.h>
// Data
static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;

static int WindowWidth = 1280;
static int WindowHeight = 800;
static bool Open = true;


// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool LoadTextureFromFile(const char*, ID3D11ShaderResourceView**, int*, int*);

// Main code
int main(int, char**)
{
    RECT desktop;
    GetWindowRect(GetDesktopWindow(), &desktop);
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"Haunekon", NULL };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, L"Haunekon", WS_POPUP, desktop.right / 2 - 1280 / 2, desktop.bottom / 2 - 800 / 2, 1280, 800, NULL, NULL, wc.hInstance, NULL);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);
    ::SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Setup Dear ImGui style
    ImGuiStyle& style = ImGui::GetStyle();
    //style.WindowRounding = 5.0;
    style.WindowPadding = ImVec2(0,0);
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    //style.Colors[ImGuiCol_TitleBg] = ImVec4(68, 196, 161, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.267f, 0.769f, 0.631f, 1.0f);
    style.Colors[ImGuiCol_Border] = ImVec4(0.267f, 0.769f, 0.631f, 1.0f);


    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImFont* font = io.Fonts->AddFontDefault();
    // Add character ranges and merge into the previous font
    // The ranges array is not copied by the AddFont* functions and is used lazily
    // so ensure it is available at the time of building or calling GetTexDataAsRGBA32().
    static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 }; // Will not be copied by AddFont* so keep in scope.
    ImFontConfig config;
    config.MergeMode = true; 
    config.GlyphMinAdvanceX = 10.0f;
    //config.GlyphMaxAdvanceX = 0.0f;
    config.GlyphOffset = ImVec2(0, 5);
    static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    io.Fonts->AddFontFromFileTTF("fontawesome-webfont.ttf", 20.0f, &config, icon_ranges);
    io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\\Arial.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesVietnamese());
    io.Fonts->Build();

    int my_image_width = 75;
    int my_image_height = 50;
    ID3D11ShaderResourceView* my_texture = NULL;
    bool ret = LoadTextureFromFile("Image/viis_logo.jpg", &my_texture, &my_image_width, &my_image_height);
    IM_ASSERT(ret);


    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const ImGuiViewport* p_vp = ImGui::GetMainViewport();

    //My Variable
    bool check = false;
    char somelogin[25] = "";
    DWORD dwFlag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;

    bool tab = false;

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (!Open) ExitProcess(EXIT_SUCCESS);

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        
        ImGui::SetNextWindowSize(p_vp->Size);
        ImGui::SetNextWindowPos(p_vp->Pos);

        ImGui::Begin("HauneKon", &Open, dwFlag); {
            const ImVec2 pos = ImGui::GetWindowPos();
            ImDrawList* draw = ImGui::GetWindowDrawList();

            if (ImGui::BeginTable("Menu", 1, NULL, ImVec2(75, 800)))
            {
                ImGui::TableSetupColumn("");
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                {
                    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0)); {
                        draw->AddRectFilled(ImVec2(0, 0), ImVec2(75, 800), ImColor(255, 255, 255));
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor(242, 242, 242));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor(217, 217, 217));
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(70, 56, 57, 255));
                        //draw->AddRectFilledMultiColor(ImVec2(0, 0), ImVec2(75, 100), IM_COL32(0, 0, 0, 100), IM_COL32(255, 0, 0, 100), IM_COL32(255, 255, 0, 100), IM_COL32(0, 255, 0, 100));
                        draw->AddRectFilled(ImVec2(0, 0), ImVec2(75, 50), ImColor(55, 255, 55, 1.0));
                        ImGui::Image((void*)my_texture, ImVec2(my_image_width, my_image_height));

                        static int selected = 0;

                        selected == 0 ? ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(242, 242, 242)) : ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
                        if (ImGui::Button(ICON_FA_HOME, ImVec2(75, 50))) {
                            selected = 0;
                        }                
                        ImGui::PopStyleColor();

                        selected == 1 ? ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(242, 242, 242)) : ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
                        if (ImGui::Button(ICON_FA_AMBULANCE,ImVec2(75, 50))) {
                            selected = 1;
                        }
                        ImGui::PopStyleColor();

                        selected == 2 ? ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(242, 242, 242)) : ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
                        if (ImGui::Button(ICON_FA_AMAZON, ImVec2(75, 50))) {
                            selected = 2;
                        }
                        ImGui::PopStyleColor();

                        selected == 3 ? ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(242, 242, 242)) : ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255));
                        if (ImGui::Button(ICON_FA_APPLE, ImVec2(75, 50))) {
                            selected = 3;
                        }
                        ImGui::PopStyleColor();
                        ImGui::PopStyleVar();
                    }
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::SetCursorPos(ImVec2(0,750));
                    ImGui::Button(ICON_FA_SIGN_OUT, ImVec2(75, 50));
                    ImGui::PopStyleColor(4);
                }
            }
            ImGui::EndTable();
            draw->AddRectFilled(ImVec2(75, 0), ImVec2(1280, 71), ImColor(255, 255, 255));
            draw->AddRectFilled(ImVec2(75, 71), ImVec2(1280, 800), ImColor(242, 242, 242));
            draw->AddRectFilled(ImVec2(75 + 10, 71 + 10), ImVec2(300, 789), ImColor(255, 255, 255), 5.0f);
            ImGui::SetCursorPos(ImVec2(75 + 60, 71 + 20));
            static float value = 0;
            ImGui::PushFont(io.Fonts->Fonts[1]);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(70, 56, 57, 255));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(242, 242, 242, 255));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(242, 242, 242, 255));
            if (ImGuiKnobs::Knob(u8"Tần số", &value, 0.0f, 50.0f, 0.1f, "%.1fdB", ImGuiKnobVariant_WiperOnly, 100.0)) {
                // value was changed
            }
            ImGui::PopFont();
            ImGui::PopStyleColor(3);
            //static int i = 0;
            //i++;
            //value = i;
            //i = i > 50 ? 0 : i;        
            
        }
        ImGui::End();
        ImGui::EndFrame();
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
        //g_pSwapChain->Present(0, 0); // Present without vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE; 
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width, int* out_height)
{
    // Load from disk into a raw RGBA buffer
    int image_width = 0;
    int image_height = 0;
    unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
    if (image_data == NULL)
        return false;

    // Create texture
    D3D11_TEXTURE2D_DESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.Width = image_width;
    desc.Height = image_height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;

    ID3D11Texture2D* pTexture = NULL;
    D3D11_SUBRESOURCE_DATA subResource;
    subResource.pSysMem = image_data;
    subResource.SysMemPitch = desc.Width * 4;
    subResource.SysMemSlicePitch = 0;
    g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

    // Create texture view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = desc.MipLevels;
    srvDesc.Texture2D.MostDetailedMip = 0;
    g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, out_srv);
    pTexture->Release();

    *out_width = image_width;
    *out_height = image_height;
    stbi_image_free(image_data);

    return true;
}