#pragma once

#include <Poco/Notification.h>

/**
 * @brief Informs the application that the user wants to quit.
 */
class QuitNotification : public Poco::Notification
{
public:
    std::string name() const override;
};
