#include "DisplayToastNotification.h"

DisplayToastNotification::DisplayToastNotification(std::string toastText)
    : _toastText(std::move(toastText))
{
}

std::string DisplayToastNotification::name() const
{
    return "DisplayToastNotification";
}

const std::string& DisplayToastNotification::ToastText() const
{
    return _toastText;
}
