#pragma once

#include <Poco/Logger.h>

#include <string>

class projectm;

/**
 * @brief WASAPI-based audio capturing thread.
 *
 * Uses the Windows Audio Session API to capture PCM data from either a loopback device or
 * any pother available input device.
 *
 * The loopback device is always considered as the "first" available device. All other external audio
 * sources come after that.
 */
class AudioCaptureImpl
{
public:

    /**
     * @brief Starts audio capturing with the first available device.
     * @param projectMHandle projectM instance handle that will receive the captured data.
     * @param audioDeviceId The initial audio device ID to capture from. Use -1 to select the implementation's
     *                      default device.
     */
    void StartRecording(projectm* projectMHandle, int audioDeviceId);

    /**
     * @brief Stops audio recording.
     */
    void StopRecording();

    /**
     * @brief Switches to the next available audio recording device.
     */
    void NextAudioDevice();

    /**
     * @brief Retrieves the current audio device name.
     * @return The name of the currently selected audio recording device.
     */
    std::string AudioDeviceName() const;

    /**
     * @brief Asks the capture client to fill projectM's audio buffer for the next frame.
     */
    void FillBuffer();

protected:

    Poco::Logger& _logger{ Poco::Logger::get("AudioCapture.SDL") }; //!< The class logger.
};


