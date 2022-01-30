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
    if (_config->getBool("listDevices", false))
    {
        std::cout << "Available audio capturing devices:" << std::endl;
        for (const auto& device: deviceList)
        {
            std::cout << "    " << device.first << " = " << device.second << std::endl;
        }
    }

    int audioDeviceId{ -1 };

    // Check if configured device is a number or string
    try
    {
        audioDeviceId = _config->getInt("device", -1);
        if (deviceList.find(audioDeviceId) == deviceList.end())
        {
            audioDeviceId = -1;
        }
    }
    catch (Poco::SyntaxException& ex)
    {
        auto audioDeviceName = _config->getString("device", "");
        for (const auto& device: deviceList)
        {
            if (device.second == audioDeviceName)
            {
                audioDeviceId = device.first;
                break;
            }
        }
    }

    poco_information_f2(_logger, R"(Recording audio from device "%s" (ID %?d).)",
                        deviceList[audioDeviceId], audioDeviceId);

    _impl->StartRecording(projectMWrapper.ProjectM(), audioDeviceId);
}

void AudioCapture::uninitialize()
{
    _impl->StopRecording();
    _impl.reset();
}

void AudioCapture::NextAudioDevice()
{
    _impl->NextAudioDevice();
}

std::string AudioCapture::AudioDeviceName() const
{
    return _impl->AudioDeviceName();
}
