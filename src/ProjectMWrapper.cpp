#include "ProjectMWrapper.h"

#include "SDLRenderingWindow.h"

#include <Poco/Util/Application.h>

#include <SDL2/SDL_opengl.h>

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

        int canvasWidth{0};
        int canvasHeight{0};

        sdlWindow.GetDrawableSize(canvasWidth, canvasHeight);

        auto presetPath = _config->getString("presetPath", app.config().getString("application.dir", ""));
        auto texturePath = _config->getString("texturePath", app.config().getString("", ""));

        _projectM = projectm_create();

        projectm_set_window_size(_projectM, canvasWidth, canvasHeight);
        projectm_set_fps(_projectM, _config->getInt("fps", 60));
        projectm_set_mesh_size(_projectM, _config->getInt("meshX", 220), _config->getInt("meshY", 125));
        projectm_set_aspect_correction(_projectM, _config->getBool("aspectCorrectionEnabled", true));

        // Preset display settings
        projectm_set_preset_duration(_projectM, _config->getInt("displayDuration", 30));
        projectm_set_soft_cut_duration(_projectM, _config->getInt("transitionDuration", 3));
        projectm_set_hard_cut_enabled(_projectM, _config->getBool("hardCutsEnabled", false));
        projectm_set_hard_cut_duration(_projectM, _config->getInt("hardCutDuration", 20));
        projectm_set_hard_cut_sensitivity(_projectM, static_cast<float>(_config->getDouble("hardCutSensitivity", 1.0)));
        projectm_set_beat_sensitivity(_projectM, static_cast<float>(_config->getDouble("beatSensitivity", 1.0)));

        if (!texturePath.empty())
        {
            const char* texturePathList[1]{&texturePath[0]};
            projectm_set_texture_search_paths(_projectM, texturePathList, 1);
        }

        // Playlist
        _playlist = projectm_playlist_create(_projectM);

        projectm_playlist_set_shuffle(_playlist, _config->getBool("shuffleEnabled", true));
        if (!presetPath.empty())
        {
            projectm_playlist_add_path(_playlist, presetPath.c_str(), true, false);
            projectm_playlist_sort(_playlist, 0, projectm_playlist_size(_playlist), SORT_PREDICATE_FILENAME_ONLY, SORT_ORDER_ASCENDING);
        }
    }
}

void ProjectMWrapper::uninitialize()
{
    if (_projectM)
    {
        projectm_destroy(_projectM);
        _projectM = nullptr;
    }

    if (_playlist)
    {
        projectm_playlist_destroy(_playlist);
        _playlist = nullptr;
    }
}

projectm_handle ProjectMWrapper::ProjectM() const
{
    return _projectM;
}

projectm_playlist_handle ProjectMWrapper::Playlist() const
{
    return _playlist;
}

int ProjectMWrapper::TargetFPS()
{
    return _config->getInt("fps", 60);
    ;
}

void ProjectMWrapper::RenderFrame() const
{
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    projectm_opengl_render_frame(_projectM);
}

void ProjectMWrapper::DisplayInitialPreset()
{
    if (!_config->getBool("enableSplash", true))
    {
        if (_config->getBool("shuffleEnabled", true))
        {
            projectm_playlist_play_next(_playlist, true);
        }
        else
        {
            projectm_playlist_set_position(_playlist, 0, true);
        }
    }
}
