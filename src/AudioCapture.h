#pragma once

#include AUDIO_IMPL_HEADER

#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

#include <memory>

/**
 * @brief Audio capturing proxy class/subsystem.
 *
 * Creates the OS-specific audio recording class and forwards the necessary calls to it.
 */
class AudioCapture : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

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
    /**
     * @brief Prints a list of available audio devices on standard output if requested by the user.
     * @param deviceList The list of available audio devices.
     */
    void PrintDeviceList(const std::map<int, std::string>&  deviceList) const;

    /**
     * @brief Returns the index of the initial audio device that should be used.
     *
     * Tries to find a device name or index specified in the user configuration and returns
     * it if either was found and is inside the device list.
     *
     * If none was found, it returns -1, which delegates default device selection to the capture API
     * implementation.
     *
     * @param deviceList The list of available audio devices.
     * @return A device index from -1 to the number of available devices minus one.
     */
    int GetInitialAudioDeviceIndex(const std::map<int, std::string>& deviceList);

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config; //!< View of the "audio" configuration subkey.

    std::unique_ptr<AudioCaptureImpl> _impl; //!< The OS-specific capture implementation.

    Poco::Logger& _logger{ Poco::Logger::get("AudioCapture") }; //!< The class logger.
};
