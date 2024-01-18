#pragma once

#include <Poco/Notification.h>

/**
 * @brief Navigates the playlist and toggles playback modes.
 */
class PlaybackControlNotification : public Poco::Notification
{
public:
    enum class Action
    {
        NextPreset,
        PreviousPreset,
        LastPreset,
        RandomPreset,
        ToggleShuffle,
        TogglePresetLocked
    };

    explicit PlaybackControlNotification(Action action, bool smoothTransition = false);

    std::string name() const override;

    Action ControlAction() const;

    bool SmoothTransition() const;

private:
    Action _action;
    bool _smoothTransition{};
};
