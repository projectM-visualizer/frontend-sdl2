#include "AudioCapture.h"

#include AUDIO_IMPL_HEADER

#include "ProjectMWrapper.h"

#include "notifications/DisplayToastNotification.h"

#include <Poco/NotificationCenter.h>

#include <Poco/Util/Application.h>

const char* AudioCapture::name() const
{
    return "Audio Capturing";
}

void AudioCapture::initialize(Poco::Util::Application& app)
{
    _config = app.config().createView("audio");

    auto& projectMWrapper = app.getSubsystem<ProjectMWrapper>();

    if (!_impl)
    {
        _impl = new AudioCaptureImpl;
    }

    auto deviceList = _impl->AudioDeviceList();
    int audioDeviceIndex = GetInitialAudioDeviceIndex(deviceList);

    PrintDeviceList(deviceList);

    _impl->StartRecording(projectMWrapper.ProjectM(), audioDeviceIndex);
}

void AudioCapture::uninitialize()
{
    if (_impl)
    {
        _impl->StopRecording();
        delete _impl;
        _impl = nullptr;
    }
}

void AudioCapture::NextAudioDevice()
{
    if (_impl)
    {
        _impl->NextAudioDevice();
        Poco::NotificationCenter::defaultCenter().postNotification(new DisplayToastNotification(_impl->AudioDeviceName()));
    }
}

void AudioCapture::AudioDeviceIndex(int index)
{
    if (_impl)
    {
        _impl->AudioDeviceIndex(index);
        Poco::NotificationCenter::defaultCenter().postNotification(new DisplayToastNotification(_impl->AudioDeviceName()));
    }
}

int AudioCapture::AudioDeviceIndex() const
{
    if (_impl)
    {
        return _impl->AudioDeviceIndex();
    }

    return -1;
}

std::string AudioCapture::AudioDeviceName() const
{
    if (!_impl)
    {
        return {};
    }

    return _impl->AudioDeviceName();
}

AudioCapture::AudioDeviceMap AudioCapture::AudioDeviceList()
{
    if (!_impl)
    {
        return {{-1, "(No audio devices available)"}};
    }

    return _impl->AudioDeviceList();
}

void AudioCapture::FillBuffer()
{
    if (!_impl)
    {
        return;
    }

    _impl->FillBuffer();
}

void AudioCapture::PrintDeviceList(const AudioDeviceMap& deviceList) const
{
    if (_config->getBool("listDevices", false))
    {
        poco_information(_logger, "Available audio capturing devices:");
        for (const auto& device : deviceList)
        {
            poco_information_f2(_logger, "    %?d = %s", device.first, device.second);
        }
    }
}

int AudioCapture::GetInitialAudioDeviceIndex(const AudioDeviceMap& deviceList)
{
    int audioDeviceIndex{ -1 };

    // Check if configured device is a number or string
    try
    {
        audioDeviceIndex = _config->getInt("device", -1);
        if (deviceList.find(audioDeviceIndex) == deviceList.end())
        {
            poco_debug(_logger,
                       "audio.device was set to a numerical value, but out of bounds. Reverting to default device.");
            audioDeviceIndex = -1;
        }
    }
    catch (Poco::SyntaxException& ex)
    {
        auto audioDeviceName = _config->getString("device", "");

        poco_debug_f1(_logger, R"(audio.device is set to non-numerical value. Searching for device name "%s".)",
                      audioDeviceName);

        for (const auto& device: deviceList)
        {
            if (device.second == audioDeviceName)
            {
                audioDeviceIndex = device.first;
                break;
            }
        }
    }

    poco_information_f2(_logger, R"(Recording audio from device "%s" (ID %?d).)",
                        deviceList.at(audioDeviceIndex), audioDeviceIndex);

    return audioDeviceIndex;
}