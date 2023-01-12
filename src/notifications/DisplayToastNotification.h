#pragma once

#include <Poco/Notification.h>

/**
 * @brief Informs the GUI subsystem to queue a new toast message.
 */
class DisplayToastNotification : public Poco::Notification
{
public:
    std::string name() const override;

    DisplayToastNotification() = delete;

    explicit DisplayToastNotification(std::string toastText);

    const std::string& ToastText() const;

private:
    std::string _toastText;
};
