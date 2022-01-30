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

protected:

    Poco::Util::AbstractConfiguration::Ptr _config; //!< View of the "audio" configuration subkey.

    std::unique_ptr<AudioCaptureImpl> _impl; //!< The OS-specific capture implementation.

    Poco::Logger& _logger{ Poco::Logger::get("AudioCapture") }; //!< The class logger.
};
