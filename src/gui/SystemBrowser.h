#pragma once

#include <string>

class SystemBrowser
{
public:
    /**
     * @brief Opens the given URL in the default browser, if the platform is supported.
     *
     * Only Windows (ShellExecute), macOS (LSOpenCFURLRef) and Linux (xdg-open) are currently supported.
     * @param url The URL to open.
     */
    static void OpenURL(const std::string& url);
};
