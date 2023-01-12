#pragma once

#include "notifications/PlaybackControlNotification.h"

#include <projectM-4/projectM.h>
#include <projectM-4/playlist.h>

#include <Poco/Logger.h>
#include <Poco/NObserver.h>

#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/Subsystem.h>

#include <memory>

class ProjectMWrapper : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    /**
     * Returns the projectM instance handle.
     * @return The projectM instance handle used to call API functions.
     */
    projectm_handle ProjectM() const;

    /**
     * Returns the playlist handle.
     * @return The plaslist handle.
     */
    projectm_playlist_handle Playlist() const;

    /**
     * Renders a single projectM frame.
     */
    void RenderFrame() const;

    int TargetFPS();

    /**
     * @brief If splash is disabled, shows the initial preset.
     * If shuffle is on, a random preset will be picked. Otherwise, the first playlist item is displayed.
     */
    void DisplayInitialPreset();

    /**
     * @brief Changes beat sensitivity by the given value.
     * @param value A positive or negative delta value.
     */
    void ChangeBeatSensitivity(float value);

private:
    /**
     * @brief projectM callback. Called whenever a preset is switched.
     * @param isHardCut True if the switch was a hard cut.
     * @param index New preset playlist index.
     * @param context Callback context, e.g. "this" pointer.
     */
    static void PresetSwitchedEvent(bool isHardCut, unsigned int index, void* context);

    void PlaybackControlNotificationHandler(const Poco::AutoPtr<PlaybackControlNotification>& notification);

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config; //!< View of the "projectM" configuration subkey.

    projectm_handle _projectM{nullptr}; //!< Pointer to the projectM instance used by the application.
    projectm_playlist_handle _playlist{nullptr}; //!< Pointer to the projectM playlist manager instance.

    Poco::NObserver<ProjectMWrapper, PlaybackControlNotification> _playbackControlNotificationObserver{*this, &ProjectMWrapper::PlaybackControlNotificationHandler};

    Poco::Logger& _logger{Poco::Logger::get("SDLRenderingWindow")}; //!< The class logger.
};
