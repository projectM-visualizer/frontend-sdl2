#include "PlaybackControlNotification.h"

PlaybackControlNotification::PlaybackControlNotification(PlaybackControlNotification::Action action, bool smoothTransition)
    : _action(action)
    , _smoothTransition(smoothTransition)
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

bool PlaybackControlNotification::SmoothTransition() const
{
    return _smoothTransition;
}
