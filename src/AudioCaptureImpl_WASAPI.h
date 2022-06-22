#pragma once

#include <Poco/Logger.h>

#include <Audioclient.h>

#include <Poco/ActiveMethod.h>
#include <Poco/Event.h>

#include <mmdeviceapi.h>
#include <string>

struct projectm;

/**
 * @brief WASAPI-based audio capturing implementation.
 *
 * Uses the Windows Audio Session API to capture audio data from either a playback device in loopback mode or
 * a recording/input device. SDL2 also has a WASAPI driver which only supports recording devices, but no playback
 * devices in loopback mode.
 *
 * The system-default playback device is always considered as the "first" available device. All other external audio
 * sources come after that, with playback devices before recording devices.
 *
 * It supports hot-plug device changes with fallback to other devices.
 */
class AudioCaptureImpl : public IMMNotificationClient
{
public:
    /**
     * Constructor.
     */
    AudioCaptureImpl();

    /**
     * Destructor.
     */
    ~AudioCaptureImpl();

    /**
     * @brief Returns a map of available recording devices.
     * @return A vector of available audio device IDs and names.
     */
    std::map<int, std::string> AudioDeviceList();

    /**
     * @brief Starts audio capturing with the first available device.
     * @param projectMHandle projectM instance handle that will receive the captured data.
     * @param audioDeviceIndex The initial audio device ID to capture from. Use -1 to select the implementation's
     *                      default device.
     */
    void StartRecording(projectm* projectMHandle, int audioDeviceIndex);

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

    /**
     * @brief Converts a widechar/unicode string to a UTF-8-encoded string
     * @param unicodeString A pointer to a widechar string
     * @return The converted string in UTF-8 encoding.
     */
    static std::string UnicodeToString(LPCWSTR unicodeString);

    HRESULT QueryInterface(const IID& riid, void** ppvObject) override;

    ULONG AddRef() override;

    ULONG Release() override;

protected:
    /**
     * @brief Represents an audio device.
     *
     * Used for easy access to some general information to use or list audio devices.
     */
    class AudioDevice
    {
    public:
        AudioDevice() = delete;
        AudioDevice(const AudioDevice& other) = delete;
        AudioDevice& operator=(const AudioDevice& other) = delete;

        /**
         * Constructor.
         * @param device Use this audio device instance to retrieve the data from. If nullptr, this
         *               will return the default device name.
         * @param isRenderDevice Set this to true if the device is a render device and will be used in loopback mode.
         */
        explicit AudioDevice(IMMDevice* device, bool isRenderDevice);

        /**
         * Move constructor
         * @param other The original class to move from.
         */
        AudioDevice(AudioDevice&& other) noexcept;

        /**
         * Destructor.
         */
        ~AudioDevice();

        /**
         * @brief Returns the opaque device ID.
         *
         * This ID can be used to get a new interface pointer from the enumerator.
         *
         * @return The device ID as a wide-char string.
         */
        LPWSTR DeviceId() const;

        /**
         * @brief Returns the human-readable device name as a UTF-8 encoded string.
         *
         * This matches the device name shown in the Windows audio mixer and any other applications.
         *
         * @return The device name in human-readable form.
         */
        std::string FriendlyName() const;

        /**
         * @brief Returns true if the device is a playback/render device.
         * @return True if the device is a playback device, false if it is a recording device.
         */
        bool IsRenderDevice() const;

    private:
        Poco::Logger& _logger{Poco::Logger::get("AudioCapture.WASAPI.AudioDevice")}; //!< The class logger.

        /**
         * @brief Returns the friendly (human readable) name of an audio device.
         * @param pMMDevice The device to retrieve the friendly name of.
         * @return Either the friendly name, or an empty string if it could not be determined.
         */
        std::string GetAudioEndpointFriendlyName(IMMDevice* pMMDevice);

        LPWSTR _deviceId{nullptr}; //!< The device ID (GUID)
        std::string _friendlyName; //!< Human-readable device name
        bool _isRenderDevice{false}; //!< If true, the device is a render device and will be opened in loopback mode.
    };

    /**
     * @brief Creates a list of currently available audio devices.
     *
     * The returned list will only contain active devices. Playback devices are listed first, then
     * recording devices. The devices are not sorted and listed as returned by the WASAPI enumerator.
     *
     * @param enumerator The enumerator interface to use.
     * @return A vector of AudioDevice entries, each representing an available audio device.
     */
    std::vector<AudioDevice> GetAudioDeviceList(IMMDeviceEnumerator* enumerator) const;

    /**
     * @brief Opens the given audio device in capture or loopback mode.
     *
     * This method will activate the device, create a capture client and also check the
     * sample format to be valid.
     *
     * @param device A pointer to the MMDevice interface to activate.
     * @param useLoopback If true, the device is opened in loopback mode.
     * @return True if the device was opened and the stream format is usable, false if an error occurred.
     */
    bool OpenAudioDevice(IMMDevice* device, bool useLoopback);

    /**
     * @brief Closes and releases the given audio device and associated interfaces.
     * @param device The audio device to close and release.
     */
    void CloseAudioDevice(IMMDevice* device);

    /**
     * @brief Main audio capture thread.
     *
     * This method is the workhorse of the audio capture implementation. Inside the thread, the selected capture device
     * is opened and then the thread will wait for the FillBuffer event to read audio data and push it into projectM.
     *
     * The capture thread also registers the MM notification callbacks, which enable us to react to hot-plug events.
     * The callback will trigger a loop restart inside the thread to reinitialize the current audio device if it has
     * been affected by such an event. The thread itself is only restarted if the user switches audio devices manually.
     */
    void CaptureThread();

    /**
     * @brief Creates a new MMDeviceEnumerator interface.
     * @return A pointer to the created MMDeviceEnumerator or nullptr if the creation failed.
     */
    IMMDeviceEnumerator* GetDeviceEnumerator() const;

    /**
     * @brief Event which is called whenever an audio device state changed.
     *
     * This event is used to detect hot-plugging (or rather unplugging) of audio devices and
     * restart the capture thread if the active device was unplugged or disabled.
     *
     * @param pwstrDeviceId The opaque device ID for the changed device.
     * @param dwNewState The new state.
     * @return Always S_OK.
     */
    HRESULT OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) override;

    /**
     * @brief Event which is called whenever the default device changed.
     *
     * This can happen if the currently active default device, e.g. a headset, is unplugged and Windows
     * switches to another device for playback.
     *
     * We ignore the default capture device, as projectM users will mostly want to visualize what
     * they hear from their default speakers.  If the playback device changes and the default device
     * is selected, the capture thread is restarted and will use the new default device without manual
     * intervention being required.
     *
     * @param flow Data flow for this default device, eRender or eCapture.
     * @param role
     * @param pwstrDefaultDeviceId
     * @return
     */
    HRESULT OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId) override;

    /**
     * @brief Event which is called if a new device was added.
     *
     * Will only be called for new, unknown devices. Replugged USB devices or jacks will not trigger this event.
     * As we're also not immediately interested in new devices, we ignore this callback.
     *
     * @param pwstrDeviceId The opaque device ID for the new device.
     * @return Always S_OK.
     */
    HRESULT OnDeviceAdded(LPCWSTR pwstrDeviceId) override;

    /**
     * @brief Event which is called if a device was fully removed.
     *
     * Will only be called for devices which are completely removed from Windows. Unplugged USB
     * devices or jacks will not trigger this event.
     * As this will also trigger an additional OnDeviceStateChanged event, we simply ignore this.
     *
     * @param pwstrDeviceId The opaque device ID for the removed device.
     * @return Always S_OK.
     */
    HRESULT OnDeviceRemoved(LPCWSTR pwstrDeviceId) override;

    /**
     * @brief Event which is called if a device property changed.
     *
     * We are not interested in the properties, so this event is currently ignored.
     * @param pwstrDeviceId The opaque device ID for the device that had a property changed.
     * @param key The property key that has changed.
     * @return Always S_OK.
     */
    HRESULT OnPropertyValueChanged(LPCWSTR pwstrDeviceId, PROPERTYKEY key) override;

    Poco::Logger& _logger{Poco::Logger::get("AudioCapture.WASAPI")}; //!< The class logger.

    projectm* _projectMHandle{nullptr}; //!< Handle if the projectM instance that will receive the audio data.
    int _currentAudioDeviceIndex{-1}; //!< Currently selected audio device index.
    IAudioClient* _audioClient{nullptr}; //!< Currently used audio client.
    IAudioCaptureClient* _audioCaptureClient{nullptr}; //!< Currently used capture client.

    LONG _referenceCount{0}; //!< COM IUnknown object reference counter

    Poco::ActiveMethod<void, void, AudioCaptureImpl> _captureThread; //!< Active method running the capture thread.
    Poco::ActiveResult<void> _captureThreadResult{new Poco::ActiveResultHolder<void>()};
    std::string _currentCaptureDeviceId; //!< Current capture device ID. USed for checking if capturing needs restarting.
    WORD _channels{0}; //!< Number of channels on the current capture device.

    std::atomic_bool _isCapturing{false}; //!< If true, capturing is running. Capture thread will exit if set to false.
    std::atomic_bool _restartCapturing{false}; //!< If true, the capture thread will stop and restart capturing without exiting.
    Poco::Event _fillBufferEvent; //!< Event which gets set if a frame is to be rendered or the capture client should exit.
    Poco::Event _bufferFilledEvent; //!< Event which gets set if the buffer has been filled.

    static constexpr char _defaultDeviceName[] = "System Default Playback Device"; //!< Display name for the default device (index -1).
};
