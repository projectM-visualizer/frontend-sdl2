#include "ProjectMGUI.h"

#include "AnonymousProFont.h"
#include "LiberationSansFont.h"
#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl2.h"

#include <Poco/NotificationCenter.h>

#include <Poco/Util/Application.h>

#include <utility>

const char* ProjectMGUI::name() const
{
    return "Preset Selection GUI";
}

void ProjectMGUI::initialize(Poco::Util::Application& app)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    Poco::Path userConfigurationDir = Poco::Path::configHome();
    userConfigurationDir.makeDirectory().append("projectM/");
    userConfigurationDir.setFileName(app.config().getString("application.baseName") + ".UI.ini");
    _uiIniFileName = userConfigurationDir.toString();

    io.IniFilename = _uiIniFileName.c_str();

    ImGui::StyleColorsDark();

    auto& renderingWindow = Poco::Util::Application::instance().getSubsystem<SDLRenderingWindow>();
    auto& projectMWrapper = Poco::Util::Application::instance().getSubsystem<ProjectMWrapper>();

    _projectMWrapper = &projectMWrapper;
    _renderingWindow = renderingWindow.GetRenderingWindow();
    _glContext = renderingWindow.GetGlContext();

    ImGui_ImplSDL2_InitForOpenGL(_renderingWindow, _glContext);
    ImGui_ImplOpenGL3_Init("#version 130");

    UpdateFontSize();

    Poco::NotificationCenter::defaultCenter().addObserver(_displayToastNotificationObserver);
}

void ProjectMGUI::uninitialize()
{
    Poco::NotificationCenter::defaultCenter().removeObserver(_displayToastNotificationObserver);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    _projectMWrapper = nullptr;
    _renderingWindow = nullptr;
    _glContext = nullptr;
}

void ProjectMGUI::UpdateFontSize()
{
    ImGuiIO& io = ImGui::GetIO();

    float dpi{96.0f}; // Use default value of 96 DPI if SDL_GetDisplayDPI doesn't return a value!
    auto displayIndex = SDL_GetWindowDisplayIndex(_renderingWindow);
    if (displayIndex < 0)
    {
        poco_debug_f1(_logger, "Could not get display index for application window: %s", std::string(SDL_GetError()));
        return;
    }

    auto result = SDL_GetDisplayDPI(displayIndex, &dpi, nullptr, nullptr);
    if (result != 0)
    {
        poco_debug_f2(_logger, "Could not get DPI info for display %?d: %s", displayIndex, std::string(SDL_GetError()));
    }

    // Only interested in changes of > 1 DPI, really.
    if (static_cast<uint32_t>(dpi) == static_cast<uint32_t>(_dpi))
    {
        return;
    }

    poco_debug_f3(_logger, "DPI change for display %?d: %hf -> %hf", displayIndex, _dpi, dpi);

    _dpi = dpi;

    ImFontConfig config;
    config.MergeMode = true;

    io.Fonts->Clear();
    _uiFont = io.Fonts->AddFontFromMemoryCompressedTTF(&AnonymousPro_compressed_data, AnonymousPro_compressed_size, floor(12.0f * (_dpi / 96.0f)));
    _toastFont = io.Fonts->AddFontFromMemoryCompressedTTF(&LiberationSans_compressed_data, LiberationSans_compressed_size, floor(20.0f * (_dpi / 96.0f)));
    io.Fonts->Build();
    ImGui_ImplOpenGL3_CreateFontsTexture();

    ImGui::GetStyle().ScaleAllSizes(1.0);
}

void ProjectMGUI::ProcessInput(const SDL_Event& event)
{
    ImGui_ImplSDL2_ProcessEvent(&event);
}

void ProjectMGUI::Toggle()
{
    _visible = !_visible;
}

void ProjectMGUI::Visible(bool visible)
{
    _visible = visible;
}

bool ProjectMGUI::Visible() const
{
    return _visible;
}

void ProjectMGUI::Draw()
{
    // Don't render UI at all if there's no need.
    if (!_toast && !_visible)
    {
        return;
    }

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    float secondsSinceLastFrame = .0f;
    if (_lastFrameTicks == 0)
    {
        _lastFrameTicks = SDL_GetTicks64();
    }
    else
    {
        auto currentFrameTicks = SDL_GetTicks64();
        secondsSinceLastFrame = static_cast<float>(currentFrameTicks - _lastFrameTicks) * .001f;
        _lastFrameTicks = currentFrameTicks;
    }

    if (_toast)
    {
        if (!_toast->Draw(secondsSinceLastFrame))
        {
            _toast.reset();
        }
    }

    if (_visible)
    {
        _mainMenu.Draw();
        _settingsWindow.Draw();
        _helpWindow.Draw();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool ProjectMGUI::WantsKeyboardInput()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureKeyboard;
}

bool ProjectMGUI::WantsMouseInput()
{
    auto& io = ImGui::GetIO();
    return io.WantCaptureMouse;
}

void ProjectMGUI::PushToastFont()
{
    ImGui::PushFont(_toastFont);
}

void ProjectMGUI::PushUIFont()
{
    ImGui::PushFont(_uiFont);
}

void ProjectMGUI::PopFont()
{
    ImGui::PopFont();
}

void ProjectMGUI::ShowSettingsWindow()
{
    _settingsWindow.Show();
}

void ProjectMGUI::ShowHelpWindow()
{
    _helpWindow.Show();
}

void ProjectMGUI::DisplayToastNotificationHandler(const Poco::AutoPtr<DisplayToastNotification>& notification)
{
    if (Poco::Util::Application::instance().config().getBool("projectM.displayToasts", true))
    {
        _toast = std::make_unique<ToastMessage>(notification->ToastText(), 3.0f);
    }
}