// Keep it as the first line, as SDL2 will otherwise redefine
// ProjectMSDLApplication::main to ProjectMSDLApplication::SDL_main
#define SDL_MAIN_HANDLED

#include "ProjectMSDLApplication.h"

#include "AudioCapture.h"
#include "ProjectMWrapper.h"
#include "RenderLoop.h"
#include "SDLRenderingWindow.h"
#include "gui/ProjectMGUI.h"

#include <Poco/Environment.h>
#include <Poco/File.h>

#include <Poco/Util/HelpFormatter.h>

#include <iostream>

ProjectMSDLApplication::ProjectMSDLApplication()
    : Poco::Util::Application()
{
    // Note: order here is important, as subsystems are initialized in the same order.
    addSubsystem(new SDLRenderingWindow);
    addSubsystem(new ProjectMWrapper);
    addSubsystem(new AudioCapture);
    addSubsystem(new ProjectMGUI);
}

const char* ProjectMSDLApplication::name() const
{
    return "projectMSDL";
}

void ProjectMSDLApplication::initialize(Poco::Util::Application& self)
{
    // Application settings are PRIO_APPLICATION, higher values have lower precedence.
    // So we put command-line overrides just below settings changed in the UI.
    config().add(_commandLineOverrides, PRIO_APPLICATION + 10);
    getSubsystem<ProjectMGUI>().CommandLineConfiguration(_commandLineOverrides);

    std::string configFileName = config().getString("application.baseName") + ".properties";
    Poco::Path userConfigurationDir = Poco::Path::configHome();
    userConfigurationDir.makeDirectory().append("projectM/");

    try
    {
        if (loadConfiguration(PRIO_DEFAULT) == 0)
        {
            // The file may be located in the ../Resources bundle dir on macOS, elsewhere relative
            // to the executable or within an absolute path.
            // By setting and retrieving the compiled-in default, we can make use of POCO's variable replacement.
            // This allows using ${application.dir} etc. in the path.
            config().setString("application.defaultConfigurationFile", PROJECTMSDL_CONFIG_LOCATION);
            std::string configPath = config().getString("application.defaultConfigurationFile", "");
            if (!configPath.empty())
            {
                Poco::Path configFilePath(configPath);
                configFilePath.makeDirectory().setFileName(configFileName);
                if (Poco::File(configFilePath).exists())
                {
                    loadConfiguration(configFilePath.toString(), PRIO_DEFAULT);
                }

            }
        }
    }
    catch (Poco::Exception& ex)
    {
        poco_error_f1(logger(), "Failed to load default configuration file: %s", ex.displayText());
    }

    try
    {
        // Try to load user's custom configuration file.
        Poco::Path userConfigurationFile = userConfigurationDir;
        userConfigurationFile.setFileName(configFileName);
        if (!Poco::File(userConfigurationFile).exists())
        {
            Poco::File(userConfigurationDir).createDirectories();
            Poco::File(userConfigurationFile).createFile();
        }
        Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> userConfiguration = new Poco::Util::PropertyFileConfiguration(userConfigurationFile.toString());
        config().add(userConfiguration, PRIO_DEFAULT - 10);

        // Pass the config data to the UI
        getSubsystem<ProjectMGUI>().UserConfiguration(userConfiguration);
    }
    catch (Poco::Exception& ex)
    {
        poco_error_f1(logger(), "Failed to load/create user configuration file: %s", ex.displayText());
    }

    // Add another layer on top for temporary command-line parameter overrides
    config().add(new Poco::Util::MapConfiguration(), PRIO_DEFAULT - 30);

    Application::initialize(self);
}

void ProjectMSDLApplication::uninitialize()
{
    Application::uninitialize();
}

void ProjectMSDLApplication::defineOptions(Poco::Util::OptionSet& options)
{
    using Poco::Util::Option;
    using Poco::Util::OptionCallback;

    options.addOption(Option("help", "h", "Display this help text and exit.")
                          .callback(
                              OptionCallback<ProjectMSDLApplication>(this, &ProjectMSDLApplication::DisplayHelp)));

    options.addOption(Option("listAudioDevices", "l",
                             "Output a list of available audio recording devices on startup.")
                          .callback(
                              OptionCallback<ProjectMSDLApplication>(this, &ProjectMSDLApplication::ListAudioDevices)));

    options.addOption(Option("audioDevice", "d",
                             "Select an audio device to record from initially. Can be the numerical ID or the full device name. "
                             "If the device is not found, the default device will be used instead.",
                             false, "<id or name>", true)
                          .binding("audio.device", _commandLineOverrides));

    options.addOption(Option("presetPath", "p", "Base directory to search for presets.",
                             false, "<path>", true)
                          .binding("projectM.presetPath", _commandLineOverrides));

    options.addOption(Option("texturePath", "", "Additional path with textures/images.",
                             false, "<path>", true)
                          .binding("projectM.texturePath", _commandLineOverrides));

    options.addOption(Option("enableSplash", "s", "If true, initially displays the built-in projectM logo preset.",
                             false, "<0/1>", true)
                          .binding("projectM.enableSplash", _commandLineOverrides));

    options.addOption(Option("fullscreen", "f", "Start in fullscreen mode.",
                             false, "<0/1>", true)
                          .binding("window.fullscreen", _commandLineOverrides));

    options.addOption(Option("exclusive", "e",
                             "Use exclusive fullscreen mode. If true, this will change display resolution on most platforms to best match the window resolution.",
                             false, "<0/1>", true)
                          .binding("window.fullscreen.exclusiveMode", _commandLineOverrides));

    options.addOption(Option("monitor", "",
                             "Displays the window on the given monitor. 0 uses OS default window position, 1 is the primary display and so on.",
                             false, "<number>", true)
                          .binding("window.monitor", _commandLineOverrides));

    options.addOption(Option("vsync", "",
                             "If true, waits for vertical sync to avoid tearing, but limits max FPS to the vsync interval.",
                             false, "<0/1>", true)
                          .binding("window.waitForVerticalSync", _commandLineOverrides));

    options.addOption(Option("width", "", "Initial window width.",
                             false, "<number>", true)
                          .binding("window.width", _commandLineOverrides));

    options.addOption(Option("height", "", "Initial window height.",
                             false, "<number>", true)
                          .binding("window.height", _commandLineOverrides));

    options.addOption(Option("fullscreenWidth", "", "Fullscreen horizontal resolution.",
                             false, "<number>", true)
                          .binding("window.fullscreen.width", _commandLineOverrides));

    options.addOption(Option("fullscreenHeight", "", "Fullscreen vertical resolution.",
                             false, "<number>", true)
                          .binding("window.fullscreen.height", _commandLineOverrides));

    options.addOption(Option("left", "", "Initial window X position.",
                             false, "<number>", true)
                          .binding("window.left", _commandLineOverrides));

    options.addOption(Option("top", "", "Initial window Y position.",
                             false, "<number>", true)
                          .binding("window.top", _commandLineOverrides));

    options.addOption(Option("fps", "", "Target frames per second rate.",
                             false, "<number>", true)
                          .binding("projectM.fps", _commandLineOverrides));

    options.addOption(Option("shuffleEnabled", "", "Shuffle enabled.",
                             false, "<0/1>", true)
                          .binding("projectM.shuffleEnabled", _commandLineOverrides));

    options.addOption(Option("presetDuration", "", "Preset duration. Any number > 1, default 30.",
                             false, "<number>", true)
                          .binding("projectM.displayDuration", _commandLineOverrides));

    options.addOption(Option("transitionDuration", "", "Transition duration. Any number >= 0, default 3.",
                             false, "<number>", true)
                          .binding("projectM.transitionDuration", _commandLineOverrides));

    options.addOption(Option("hardCutsEnabled", "", "Hard cuts enabled.",
                             false, "<0/1>", true)
                          .binding("projectM.hardCutsEnabled", _commandLineOverrides));

    options.addOption(Option("hardCutDuration", "", "Hard cut duration. Any number > 1, default 20.",
                             false, "<number>", true)
                          .binding("projectM.hardCutDuration", _commandLineOverrides));

    options.addOption(Option("hardCutSensitivity", "", "Hard cut sensitivity. Between 0.0 and 5.0. Default 1.0.",
                             false, "<number>", true)
                          .binding("projectM.hardCutSensitivity", _commandLineOverrides));

    options.addOption(Option("beatSensitivity", "", "Beat sensitivity. Between 0.0 and 2.0. Default 1.0.",
                             false, "<number>", true)
                          .binding("projectM.beatSensitivity", _commandLineOverrides));
}

int ProjectMSDLApplication::main(POCO_UNUSED const std::vector<std::string>& args)
{
    RenderLoop renderLoop;
    renderLoop.Run();

    return EXIT_SUCCESS;
}

void ProjectMSDLApplication::DisplayHelp(POCO_UNUSED const std::string& name, POCO_UNUSED const std::string& value)
{
    Poco::Util::HelpFormatter formatter(options());

    SDL_version sdlBuild;
    SDL_version sdlLoaded;

    SDL_VERSION(&sdlBuild);
    SDL_GetVersion(&sdlLoaded);

    auto* projectMVersion = projectm_get_version_string();
    std::string projectMRuntimeVersion(projectMVersion);
    projectm_free_string(projectMVersion);

    formatter.setUsage(config().getString("application.name") + " [options]");
    formatter.setHeader(Poco::format(R"(
projectM SDL Standalone Visualizer

Licensed under the GNU General Public License 3.0

Application version: %s
Built/running with projectM4 version: %s / %s
Built against SDL version: %?d.%?d.%?d (running with %?d.%?d.%?d))",
                                     std::string(PROJECTMSDL_VERSION),
                                     std::string(PROJECTM_VERSION_STRING),
                                     projectMRuntimeVersion,
                                     sdlBuild.major, sdlBuild.minor, sdlBuild.patch,
                                     sdlLoaded.major, sdlLoaded.minor, sdlLoaded.patch));

    formatter.format(std::cerr);

    exit(EXIT_SUCCESS);
}

void ProjectMSDLApplication::ListAudioDevices(POCO_UNUSED const std::string& name, POCO_UNUSED const std::string& value)
{
    _commandLineOverrides->setBool("audio.listDevices", true);
}
