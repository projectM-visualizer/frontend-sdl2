#include "SystemBrowser.h"

#ifdef __MSC_VER__
#include <shellapi.h>
#include <windows.h>
#endif
#ifdef __linux__
#include <Poco/Process.h>
#endif
#ifdef __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CFBundle.h>
#endif

void SystemBrowser::OpenURL(const std::string& url)
{
#ifdef __MSC_VER__
    ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
#endif
#ifdef __linux__
    auto handle = Poco::Process::launch("xdg-open", {url});
    handle.wait();
#endif
#ifdef __APPLE__
    CFURLRef urlRef = CFURLCreateWithBytes(nullptr, reinterpret_cast<const UInt8*>(url.c_str()), url.length(), kCFStringEncodingASCII, nullptr);
    LSOpenCFURLRef(urlRef, 0);
    CFRelease(urlRef);
#endif
}
