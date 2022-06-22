#include "AudioCapture.h"

#include "ProjectMWrapper.h"

#include <Poco/Util/Application.h>

const char* AudioCapture::name() const
{
    return "Audio Capturing";
}

void AudioCapture::initialize(Poco::Util::Application& app)
{
    _config = app.config().createView("audio");

    auto& projectMWrapper = app.getSubsystem<ProjectMWrapper>();

    _impl = std::make_unique<AudioCaptureImpl>();

    auto deviceList = _impl->AudioDeviceList();
    int audioDeviceIndex = GetInitialAudioDeviceIndex(deviceList);

    PrintDeviceList(deviceList);

    _impl->StartRecording(projectMWrapper.ProjectM(), audioDeviceIndex);
}

void AudioCapture::uninitialize()
{
    _impl->StopRecording();
    _impl.reset();
}

void AudioCapture::NextAudioDevice()
{
    if (_impl)
    {
        _impl->NextAudioDevice();
    }
}

std::string AudioCapture::AudioDeviceName() const
{
    if (!_impl)
    {
        return {};
    }

    return _impl->AudioDeviceName();
}

void AudioCapture::FillBuffer()
{
    if (!_impl)
    {
        return;
    }

    _impl->FillBuffer();
}

void AudioCapture::PrintDeviceList(const std::map<int, std::string>& deviceList) const
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

int AudioCapture::GetInitialAudioDeviceIndex(const std::map<int, std::string>& deviceList)
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