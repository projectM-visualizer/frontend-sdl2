#include "PlaybackControlNotification.h"

PlaybackControlNotification::PlaybackControlNotification(PlaybackControlNotification::Action action)
    : _action(action)
{
}

std::string PlaybackControlNotification::name() const
{
    return "PlaybackControlNotification";
}

PlaybackControlNotification::Action PlaybackControlNotification::ControlAction() const
{
    return _action;
}