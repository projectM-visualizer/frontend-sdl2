#include "ProjectMWrapper.h"

#include "SDLRenderingWindow.h"

#include <Poco/File.h>
#include <Poco/Util/Application.h>

#include <SDL2/SDL_opengl.h>
#include <fstream>

const char* ProjectMWrapper::name() const
{
    return "ProjectM Wrapper";
}

void ProjectMWrapper::initialize(Poco::Util::Application& app)
{
    _config = app.config().createView("projectM");

    if (!_projectM)
    {
        auto& sdlWindow = app.getSubsystem<SDLRenderingWindow>();

        int canvasWidth{ 0 };
        int canvasHeight{ 0 };

        sdlWindow.GetDrawableSize(canvasWidth, canvasHeight);

        auto presetPath = _config->getString("presetPath", app.config().getString("application.dir", ""));
        auto presetFilter = _config->getString("projectM.presetFilter", "");
        auto presetExcludeFile = _config->getString("projectM.presetExcludeFile");
        auto texturePath = _config->getString("texturePath", app.config().getString("", ""));

        projectm_settings settings{};

        // Window/rendering settings
        settings.window_width = canvasWidth;
        settings.window_height = canvasHeight;
        settings.fps = _config->getInt("fps", 60);
        settings.mesh_x = _config->getInt("meshX", 220);
        settings.mesh_y = _config->getInt("meshY", 125);
        settings.aspect_correction = _config->getBool("aspectCorrectionEnabled", true);

        // Preset display settings
        settings.preset_duration = _config->getInt("displayDuration", 30);
        settings.soft_cut_duration = _config->getInt("transitionDuration", 3);
        settings.hard_cut_enabled = _config->getBool("hardCutsEnabled", false);
        settings.hard_cut_duration = _config->getInt("hardCutDuration", 20);
        settings.hard_cut_sensitivity = static_cast<float>(_config->getDouble("hardCutSensitivity", 1.0));
        settings.beat_sensitivity = static_cast<float>(_config->getDouble("beatSensitivity", 1.0));
        settings.shuffle_enabled = _config->getBool("shuffleEnabled", true);
        if (!presetPath.empty())
        {
            settings.preset_path = &presetPath[0];
        }
        if (!texturePath.empty())
        {
            settings.texture_path = &texturePath[0];
        }

        // Unsupported settings
        settings.soft_cut_ratings_enabled = false;

        _projectM = projectm_create_settings(&settings, PROJECTM_FLAG_NONE);

        // remove any presets that don't include filter
        if (presetFilter != "") {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset include filter '%s'\n", presetFilter.c_str());

            unsigned int size = projectm_get_playlist_size(_projectM);
            for (int i = size; i > 0; i--) {
                std::string presetName(projectm_get_preset_name(_projectM, i-1));
                unsigned int presetIndex = projectm_get_preset_index(_projectM, presetName.c_str());
                std::string presetFileName(projectm_get_preset_filename(_projectM, presetIndex));

                if (presetFileName.find(presetFilter) == std::string::npos) {
                    projectm_remove_preset(_projectM, presetIndex);
                } else {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset include '%s'\n", presetFileName.c_str());
                }
            }
        }

        // remove any presets in exclude.txt
        Poco::Path excludeFile = Poco::Path(presetExcludeFile);
        if (Poco::File(excludeFile).exists()) {
            SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset exclude config '%s'\n", excludeFile.toString().c_str());

            // TODO: use Poco to read file?
            // Poco::FileInputStream sin(excludeFile.toString());
            // Poco::InputLineEndingConverter lin(sin);
            // std::string line;
            // line = lin.getNewLine();

            std::ifstream infile(excludeFile.toString());
            std::string line;
            for( std::string line; getline( infile, line ); )
            {
                unsigned int size = projectm_get_playlist_size(_projectM);
                unsigned int index = projectm_get_preset_index(_projectM, line.c_str());
                if (index < size) {
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Preset exclude '%s'\n", line.c_str());
                    projectm_remove_preset(_projectM, index);
                }
            }
        }

        if (!_config->getBool("enableSplash", true))
        {
            if (settings.shuffle_enabled)
            {
                projectm_select_random_preset(_projectM, true);
            }
            else
            {
                projectm_select_next_preset(_projectM, true);
            }
        }

        SetHelpText();
    }
}

void ProjectMWrapper::uninitialize()
{
    if (_projectM)
    {
        projectm_destroy(_projectM);
        _projectM = nullptr;
    }
}

projectm* ProjectMWrapper::ProjectM() const
{
    return _projectM;
}

int ProjectMWrapper::TargetFPS()
{
    return _config->getInt("fps", 60);;
}

void ProjectMWrapper::RenderFrame() const
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_render_frame(_projectM);
}

void ProjectMWrapper::SetHelpText()
{
    // Create a help menu specific to SDL
    std::string modKey = "CTRL";

#if __APPLE__
    modKey = "CMD";
#endif

    std::string helpText = "\n"
                           "F1: This help menu\n"
                           "F3: Show preset name\n"
                           "F4: Show details and statistics\n"
                           "F5: Show FPS\n"
                           "ENTER: Open preset search menu\n"
                           "SPACE: Lock/unlock preset\n"
                           "R: Random preset\n"
                           "M: Open preset list\n"
                           "N: Next preset\n"
                           "P: Previous preset\n"
                           "UP: Increase beat sensitivity\n"
                           "DOWN: Decrease beat sensitivity\n"
                           "\n"
                           "SHIFT+Left Click: Add random waveform\n"
                           "Middle Click: Remove all random waveforms\n"
                           "Right Click: Toggle fullscreen\n"
                           "\n" +
                           modKey + "+I: Next audio input device\n" +
                           modKey + "+M: Change monitor\n" +
                           modKey + "+F: Toggle fullscreen\n" +
                           modKey + "+Q: Quit projectM";
}
