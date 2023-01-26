#include "ProjectMWrapper.h"

#include "SDLRenderingWindow.h"

#include "notifications/DisplayToastNotification.h"

#include <Poco/NotificationCenter.h>

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

        auto presetPaths = GetPathListWithDefault("presetPath", app.config().getString("application.dir", ""));
        auto texturePaths = GetPathListWithDefault("texturePath", app.config().getString("", ""));

        _projectM = projectm_create();
        if (_projectM == nullptr)
        {
            poco_fatal(_logger, "Could not create projectM instance!");
            throw std::runtime_error("Could not create projectM instance!");
        }

        projectm_set_window_size(_projectM, canvasWidth, canvasHeight);
        projectm_set_fps(_projectM, _config->getInt("fps", 60));
        projectm_set_mesh_size(_projectM, _config->getInt("meshX", 220), _config->getInt("meshY", 125));
        projectm_set_aspect_correction(_projectM, _config->getBool("aspectCorrectionEnabled", true));
        projectm_set_preset_locked(_projectM, _config->getBool("presetLocked", false));

        // Preset display settings
        projectm_set_preset_duration(_projectM, _config->getInt("displayDuration", 30));
        projectm_set_soft_cut_duration(_projectM, _config->getInt("transitionDuration", 3));
        projectm_set_hard_cut_enabled(_projectM, _config->getBool("hardCutsEnabled", false));
        projectm_set_hard_cut_duration(_projectM, _config->getInt("hardCutDuration", 20));
        projectm_set_hard_cut_sensitivity(_projectM, static_cast<float>(_config->getDouble("hardCutSensitivity", 1.0)));
        projectm_set_beat_sensitivity(_projectM, static_cast<float>(_config->getDouble("beatSensitivity", 1.0)));

        if (!texturePaths.empty())
        {
            std::vector<const char*> texturePathList;
            texturePathList.reserve(texturePaths.size());
            for (const auto& texturePath : texturePaths)
            {
                texturePathList.push_back(texturePath.data());
            }

            projectm_set_texture_search_paths(_projectM, texturePathList.data(), texturePaths.size());
        }

        // Playlist
        _playlist = projectm_playlist_create(_projectM);
        if (_playlist == nullptr)
        {
            poco_fatal(_logger, "Could not create projectM playlist manager!");
            throw std::runtime_error("Could not create projectM playlist manager!");
        }

        projectm_playlist_set_shuffle(_playlist, _config->getBool("shuffleEnabled", true));

        for (const auto& presetPath : presetPaths)
        {
            projectm_playlist_add_path(_playlist, presetPath.c_str(), true, false);
        }
        projectm_playlist_sort(_playlist, 0, projectm_playlist_size(_playlist), SORT_PREDICATE_FILENAME_ONLY, SORT_ORDER_ASCENDING);

        projectm_playlist_set_preset_switched_event_callback(_playlist, &ProjectMWrapper::PresetSwitchedEvent, static_cast<void*>(this));
    }

    Poco::NotificationCenter::defaultCenter().addObserver(_playbackControlNotificationObserver);
}

void ProjectMWrapper::uninitialize()
{

    Poco::NotificationCenter::defaultCenter().removeObserver(_playbackControlNotificationObserver);

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

void ProjectMWrapper::ChangeBeatSensitivity(float value)
{
    projectm_set_beat_sensitivity(_projectM, projectm_get_beat_sensitivity(_projectM) + value);
    Poco::NotificationCenter::defaultCenter().postNotification(
        new DisplayToastNotification(Poco::format("Beat Sensitivity: %.2hf", projectm_get_beat_sensitivity(_projectM))));
}

void ProjectMWrapper::PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context)
{
    auto that = reinterpret_cast<ProjectMWrapper*>(context);
    auto presetName = projectm_playlist_item(that->_playlist, index);
    poco_information_f1(that->_logger, "Displaying preset: %s", std::string(presetName));
    projectm_free_string(presetName);

    Poco::NotificationCenter::defaultCenter().postNotification(new UpdateWindowTitleNotification);
}

void ProjectMWrapper::PlaybackControlNotificationHandler(const Poco::AutoPtr<PlaybackControlNotification>& notification)
{
    switch (notification->ControlAction())
    {
        case PlaybackControlNotification::Action::NextPreset:
            projectm_playlist_play_next(_playlist, true);
            break;

        case PlaybackControlNotification::Action::PreviousPreset:
            projectm_playlist_play_previous(_playlist, true);
            break;

        case PlaybackControlNotification::Action::LastPreset:
            projectm_playlist_play_last(_playlist, true);
            break;

        case PlaybackControlNotification::Action::RandomPreset: {
            bool shuffleEnabled = projectm_playlist_get_shuffle(_playlist);
            projectm_playlist_set_shuffle(_playlist, true);
            projectm_playlist_play_next(_playlist, true);
            projectm_playlist_set_shuffle(_playlist, shuffleEnabled);
            break;
        }

        case PlaybackControlNotification::Action::ToggleShuffle:
            projectm_playlist_set_shuffle(_playlist, !projectm_playlist_get_shuffle(_playlist));
            _config->setBool("shuffleEnabled", projectm_playlist_get_shuffle(_playlist));
            break;

        case PlaybackControlNotification::Action::TogglePresetLocked: {
            bool locked = !projectm_get_preset_locked(_projectM);
            projectm_set_preset_locked(_projectM, locked);
            _config->setBool("presetLocked", locked);
            Poco::NotificationCenter::defaultCenter().postNotification(new UpdateWindowTitleNotification);
            break;
        }
    }
}

std::vector<std::string> ProjectMWrapper::GetPathListWithDefault(const std::string& baseKey, const std::string& defaultPath)
{
    using Poco::Util::AbstractConfiguration;

    std::vector<std::string> pathList;
    auto defaultPresetPath = _config->getString(baseKey, defaultPath);
    if (!defaultPresetPath.empty())
    {
        pathList.push_back(defaultPresetPath);
    }
    AbstractConfiguration::Keys subKeys;
    _config->keys(baseKey, subKeys);
    for (const auto& key : subKeys)
    {
        pathList.push_back(_config->getString(baseKey + "." + key, ""));
    }
    return pathList;
}
