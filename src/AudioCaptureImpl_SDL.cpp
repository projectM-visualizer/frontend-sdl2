#include "AudioCaptureImpl_SDL.h"

#include <libprojectM/projectM.h>

#include <assert.h>

AudioCaptureImpl::AudioCaptureImpl()
{
#ifdef SDL_HINT_AUDIO_INCLUDE_MONITORS
    SDL_SetHint(SDL_HINT_AUDIO_INCLUDE_MONITORS, "1");
#endif
    SDL_InitSubSystem(SDL_INIT_AUDIO);
}

AudioCaptureImpl::~AudioCaptureImpl()
{
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

std::map<int, std::string> AudioCaptureImpl::AudioDeviceList()
{
    std::map<int, std::string> deviceList{
        { -1, "System default capturing device" }
    };

    auto recordingDeviceCount = SDL_GetNumAudioDevices(true);

    for (int i = 0; i < recordingDeviceCount; i++)
    {
        deviceList.insert(std::make_pair(i, SDL_GetAudioDeviceName(i, true)));
    }

    return deviceList;
}

void AudioCaptureImpl::StartRecording(projectm* projectMHandle, int audioDeviceIndex)
{
    _projectMHandle = projectMHandle;
    _currentAudioDeviceIndex = audioDeviceIndex;

    if (OpenAudioDevice())
    {
        SDL_PauseAudioDevice(_currentAudioDeviceID, false);
    }
}

void AudioCaptureImpl::StopRecording()
{
    if (_currentAudioDeviceID)
    {
        SDL_PauseAudioDevice(_currentAudioDeviceID, true);
        SDL_CloseAudioDevice(_currentAudioDeviceID);
        _currentAudioDeviceID = 0;
    }
}

void AudioCaptureImpl::NextAudioDevice()
{
    StopRecording();

    // Will not go back to default capture device if toggled manually.
    int nextAudioDeviceId = (_currentAudioDeviceIndex + 1) % SDL_GetNumAudioDevices(true);

    StartRecording(_projectMHandle, nextAudioDeviceId);
}

std::string AudioCaptureImpl::AudioDeviceName() const
{
    return SDL_GetAudioDeviceName(_currentAudioDeviceIndex, true);
}

bool AudioCaptureImpl::OpenAudioDevice()
{
    SDL_AudioSpec requestedSpecs{};
    SDL_AudioSpec actualSpecs{};

    requestedSpecs.freq = 44100;
    requestedSpecs.format = AUDIO_F32;
    requestedSpecs.channels = 2;
    requestedSpecs.samples = projectm_pcm_get_max_samples();
    requestedSpecs.callback = AudioCaptureImpl::AudioInputCallback;
    requestedSpecs.userdata = this;

    // Will be NULL on error, which happens if the requested index is -1. This automatically selects the default device.
    auto deviceName = SDL_GetAudioDeviceName(_currentAudioDeviceIndex, true);
    _currentAudioDeviceID = SDL_OpenAudioDevice(deviceName, true, &requestedSpecs, &actualSpecs, 0);

    if (_currentAudioDeviceID == 0)
    {
        poco_error_f3(_logger, R"(Failed to open audio device "%s" (ID %?d): %s)",
                      std::string(deviceName ? deviceName : "System default capturing device"),
                      _currentAudioDeviceIndex,
                      std::string(SDL_GetError()));
        return false;
    }

    _channels = actualSpecs.channels;

    poco_information_f4(_logger, R"(Opened audio recording device "%s" (ID %?d) with %?d channels at %?d Hz.)",
                        std::string(deviceName ? deviceName : "System default capturing device"),
                        _currentAudioDeviceIndex,
                        actualSpecs.channels,
                        actualSpecs.freq);

    return true;
}

void AudioCaptureImpl::AudioInputCallback(void* userData, unsigned char* stream, int len)
{
    assert(userData);
    auto instance = reinterpret_cast<AudioCaptureImpl*>(userData);

    unsigned int samples = len / sizeof(float) / instance->_channels;

    if (instance->_channels == 1)
    {
        projectm_pcm_add_float_1ch_data(instance->_projectMHandle, reinterpret_cast<float*>(stream), samples);
    }
    else if (instance->_channels == 2)
    {
        projectm_pcm_add_float_2ch_data(instance->_projectMHandle, reinterpret_cast<float*>(stream), samples);
    }
}
