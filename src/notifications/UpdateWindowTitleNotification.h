#pragma once

#include <Poco/Notification.h>

/**
 * @brief Informs the application that the window title should be updated.
 */
class UpdateWindowTitleNotification : public Poco::Notification
{
public:
    std::string name() const override;
};
