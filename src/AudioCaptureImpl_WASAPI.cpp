#include "AudioCaptureImpl_WASAPI.h"

#include <projectM-4/projectM.h>

#include <Poco/UnicodeConverter.h>

#include <functiondiscoverykeys_devpkey.h>
#include <mmdeviceapi.h>
#include <objbase.h>

AudioCaptureImpl::AudioCaptureImpl()
    : _captureThread(this, &AudioCaptureImpl::CaptureThread)
{
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);
}

AudioCaptureImpl::~AudioCaptureImpl()
{
    CoUninitialize();
}

std::map<int, std::string> AudioCaptureImpl::AudioDeviceList()
{
    std::map<int, std::string> deviceList{
        {-1, _defaultDeviceName}};

    IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};
    auto captureDevices{GetAudioDeviceList(enumerator)};
    enumerator->Release();

    uint32_t index{0};
    for (const auto& device : captureDevices)
    {
        if (device.DeviceId() != nullptr)
        {
            deviceList.insert(std::make_pair(index, device.FriendlyName()));
        }

        index++;
    }

    return deviceList;
}

void AudioCaptureImpl::StartRecording(projectm* projectMHandle, int audioDeviceIndex)
{
    _projectMHandle = projectMHandle;
    _currentAudioDeviceIndex = audioDeviceIndex;

    _isCapturing = true;
    _captureThreadResult = _captureThread();
}

void AudioCaptureImpl::StopRecording()
{
    if (_isCapturing)
    {
        poco_trace(_logger, "Stopping audio capturing thread.");
        _isCapturing = false;
        _fillBufferEvent.set();
        _captureThreadResult.wait();
        poco_trace(_logger, "Audio capturing thread joined.");
    }
}

void AudioCaptureImpl::NextAudioDevice()
{
    StopRecording();

    IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};
    auto captureDevices{GetAudioDeviceList(enumerator)};
    enumerator->Release();

    // Will wrap around to loopback capture device (-1).
    int nextAudioDeviceId = ((_currentAudioDeviceIndex + 2) % (static_cast<int>(captureDevices.size()) + 1)) - 1;

    StartRecording(_projectMHandle, nextAudioDeviceId);
}

std::string AudioCaptureImpl::AudioDeviceName() const
{
    if (_currentAudioDeviceIndex < 0)
    {
        return "System Default Audio Device";
    }

    IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};
    auto captureDevices{GetAudioDeviceList(enumerator)};
    enumerator->Release();

    if (captureDevices.empty() || _currentAudioDeviceIndex >= static_cast<int>(captureDevices.size()))
    {
        return {};
    }

    return captureDevices.at(_currentAudioDeviceIndex).FriendlyName();
}

void AudioCaptureImpl::FillBuffer()
{
    if (_isCapturing)
    {
        _bufferFilledEvent.reset();
        _fillBufferEvent.set();
        try
        {
            _bufferFilledEvent.wait(20);
        }
        catch (Poco::TimeoutException& ex)
        {
            poco_debug(_logger, "Timeout waiting for audio buffer fill");
        }
    }
}

HRESULT AudioCaptureImpl::QueryInterface(const IID& riid, void** ppvObject)
{
    if (ppvObject == nullptr)
    {
        return E_POINTER;
    }

    if (riid == IID_IUnknown)
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
    }
    else if (riid == __uuidof(IMMNotificationClient))
    {
        *ppvObject = static_cast<IMMNotificationClient*>(this);
        AddRef();
    }

    return E_NOINTERFACE;
}

ULONG AudioCaptureImpl::AddRef()
{
    return InterlockedIncrement(&_referenceCount);
}

ULONG AudioCaptureImpl::Release()
{
    return InterlockedDecrement(&_referenceCount);
}

std::string AudioCaptureImpl::UnicodeToString(LPCWSTR unicodeString)
{
    std::string utf8String;
    Poco::UnicodeConverter::convert(std::wstring(unicodeString), utf8String);
    return utf8String;
}

std::vector<AudioCaptureImpl::AudioDevice> AudioCaptureImpl::GetAudioDeviceList(IMMDeviceEnumerator* enumerator) const
{
    auto addEndpoints = [this, enumerator](std::vector<AudioDevice>& deviceList, EDataFlow dataFlow) {
        HRESULT result{S_OK};
        IMMDeviceCollection* audioEndpoints{nullptr};

        result = enumerator->EnumAudioEndpoints(dataFlow, DEVICE_STATE_ACTIVE, &audioEndpoints);
        if (FAILED(result))
        {
            poco_error_f1(_logger, "IMMDeviceEnumerator::EnumAudioEndpoints failed: result = 0x%08?x", result);
            return;
        }

        UINT deviceCount{0};
        audioEndpoints->GetCount(&deviceCount);
        for (UINT item = 0; item < deviceCount; item++)
        {
            IMMDevice* device{nullptr};
            result = audioEndpoints->Item(item, &device);

            if (FAILED(result) || device == nullptr)
            {
                poco_error_f2(_logger, "IMMDeviceEnumerator::Item failed for device %?u: result = 0x%08?x", item, result);
                continue;
            }

            deviceList.emplace_back(device, (dataFlow == eRender));
            device->Release();
        }

        audioEndpoints->Release();
    };

    std::vector<AudioDevice> deviceList;
    addEndpoints(deviceList, eRender);
    addEndpoints(deviceList, eCapture);

    return deviceList;
}

bool AudioCaptureImpl::OpenAudioDevice(IMMDevice* device, bool useLoopback)
{
    // activate an IAudioClient
    HRESULT result = device->Activate(__uuidof(IAudioClient),
                              CLSCTX_ALL,
                              nullptr,
                              reinterpret_cast<void**>(&_audioClient));
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IMMDevice::Activate(IAudioClient) failed: result = 0x%08?x", result);
        return false;
    }

    // get the default device periodicity
    REFERENCE_TIME hnsDefaultDevicePeriod;
    result = _audioClient->GetDevicePeriod(&hnsDefaultDevicePeriod,
                                           nullptr);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetDevicePeriod failed: result = 0x%08?x", result);
        return false;
    }

    // get the default device format
    WAVEFORMATEX* pwfx;
    result = _audioClient->GetMixFormat(&pwfx);
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient::GetMixFormat failed: result = 0x%08?x", result);
        return false;
    }

    // Should default to float32 data, but some devices might deliver other formats.
    if (pwfx->wFormatTag != WAVE_FORMAT_IEEE_FLOAT)
    {
        auto extensibleFormat = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
        if (pwfx->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        {
            poco_error_f1(_logger, "IAudioClient::GetMixFormat returned non-float sample format: 0x%04?x", pwfx->wFormatTag);
            CoTaskMemFree(pwfx);
            return false;
        }
        else if (!IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, extensibleFormat->SubFormat))
        {
            poco_error_f1(_logger, "IAudioClient::GetMixFormat returned non-float extensible sub format: 0x%04?x", extensibleFormat->SubFormat);
            CoTaskMemFree(pwfx);
            return false;
        }
    }

    _channels = pwfx->nChannels;

    // Can't use event-driven processing in loopback mode, but as we
    // get a "fill buffer" request before rendering each frame, this isn't
    // really necessary anyway.
    result = _audioClient->Initialize(
        AUDCLNT_SHAREMODE_SHARED,
        useLoopback ? AUDCLNT_STREAMFLAGS_LOOPBACK : 0,
        hnsDefaultDevicePeriod,
        0,
        pwfx,
        nullptr);

    CoTaskMemFree(pwfx);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient->Initialize failed: result = 0x%08?x", result);
        return false;
    }

    // activate an IAudioCaptureClient
    result = _audioClient->GetService(
        __uuidof(IAudioCaptureClient),
        (void**) &_audioCaptureClient);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient->GetService failed: result = 0x%08?x", result);
        return false;
    }

    result = _audioClient->Start();
    if (FAILED(result))
    {
        poco_error_f1(_logger, "IAudioClient->Start failed: result = 0x%08?x", result);
        return false;
    }

    return true;
}

void AudioCaptureImpl::CloseAudioDevice(IMMDevice* device)
{
    poco_trace(_logger, "Stopping audio client.");
    _audioClient->Stop();

    if (_audioCaptureClient)
    {
        poco_trace(_logger, "Releasing audio capture client.");
        _audioCaptureClient->Release();
        _audioCaptureClient = nullptr;
    }
    if (_audioClient)
    {
        poco_trace(_logger, "Releasing audio client.");
        _audioClient->Release();
        poco_trace(_logger, "Audio client released.");
        _audioClient = nullptr;
    }

    if (device)
    {
        poco_trace(_logger, "Releasing audio device.");
        device->Release();
    }
}

void AudioCaptureImpl::CaptureThread()
{
    poco_debug(_logger, "Audio capture thread starting.");

    HRESULT result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "CoInitializeEx() failed: result = 0x%08?x", result);
        throw std::bad_alloc();
    }

    IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};

    if (enumerator == nullptr)
    {
        CoUninitialize();
        throw std::bad_alloc();
    }

    poco_trace(_logger, "Registering device callbacks.");
    enumerator->RegisterEndpointNotificationCallback(this);

    do
    {
        _restartCapturing = false;

        auto devices{GetAudioDeviceList(enumerator)};
        bool useLoopback{true};
        std::string deviceName;

        IMMDevice* device{nullptr};

        if (_currentAudioDeviceIndex == -1 || _currentAudioDeviceIndex >= devices.size())
        {
            // Get the default render endpoint for opening it as a loopback device.
            result = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
            if (FAILED(result))
            {
                poco_error_f1(_logger, "IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: result = 0x%08?x", result);
                break;
            }

            deviceName = _defaultDeviceName;
        }
        else
        {
            // Get a device by its ID according to the currently selected index.
            useLoopback = devices.at(_currentAudioDeviceIndex).IsRenderDevice();
            deviceName = devices.at(_currentAudioDeviceIndex).FriendlyName();
            result = enumerator->GetDevice(devices.at(_currentAudioDeviceIndex).DeviceId(), &device);

            if (FAILED(result))
            {
                poco_error_f1(_logger, "IMMDeviceEnumerator::GetDevice failed: result = 0x%08?x", result);
                break;
            }
        }

        LPWSTR deviceID{nullptr};
        result = device->GetId(&deviceID);
        if (FAILED(result))
        {
            poco_error_f1(_logger, "IMMDevice::GetId failed: result = 0x%08?x", result);
        }
        else
        {
            _currentCaptureDeviceId = UnicodeToString(deviceID);
        }

        if (!OpenAudioDevice(device, useLoopback))
        {
            _isCapturing = false;
        }

        poco_information_f3(_logger, "Audio device opened: %s (channels: %hu, loopback: %b)", deviceName, _channels, useLoopback);

        while (_isCapturing && !_restartCapturing)
        {
            try
            {
                _fillBufferEvent.tryWait(500);
            }
            catch (Poco::TimeoutException& ex)
            {
                poco_debug(_logger, "FillBuffer event timeout, proceeding to flush buffer or abort.");
            }

            if (!_isCapturing)
            {
                break;
            }

            UINT32 packetLength;

            _audioCaptureClient->GetNextPacketSize(&packetLength);
            while (packetLength != 0)
            {
                BYTE* data;
                UINT32 framesAvailable;
                DWORD flags;

                result = _audioCaptureClient->GetBuffer(&data, &framesAvailable, &flags, nullptr, nullptr);
                if (FAILED(result))
                {
                    poco_error_f1(_logger, "IAudioCaptureClient::GetBuffer failed: result = 0x%08?x", result);
                    _isCapturing = false;
                    break;
                }

                if (flags & AUDCLNT_BUFFERFLAGS_SILENT)
                {
                    data = nullptr;
                }

                poco_trace_f1(_logger, "Audio frames available for capturing: %u", data ? framesAvailable : 0);

                if (framesAvailable > 0 && data != nullptr)
                {
                    projectm_pcm_add_float(_projectMHandle, reinterpret_cast<float*>(data), framesAvailable, static_cast<projectm_channels>(_channels));
                }

                _audioCaptureClient->ReleaseBuffer(framesAvailable);

                _audioCaptureClient->GetNextPacketSize(&packetLength);
            }

            _bufferFilledEvent.set();
        }

        CloseAudioDevice(device);

        poco_debug(_logger, "Audio device closed.");
    } while (_restartCapturing);

    poco_trace(_logger, "Unregistering device callbacks.");
    enumerator->UnregisterEndpointNotificationCallback(this);
    poco_trace(_logger, "Releasing audio device enumerator.");
    enumerator->Release();
    poco_trace(_logger, "Audio device enumerator released.");

    CoUninitialize();

    poco_debug(_logger, "Audio capture thread exiting.");
}

IMMDeviceEnumerator* AudioCaptureImpl::GetDeviceEnumerator() const
{
    IMMDeviceEnumerator* enumerator{nullptr};

    HRESULT result = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_ALL,
        __uuidof(IMMDeviceEnumerator),
        (void**) &enumerator);

    if (FAILED(result))
    {
        poco_error_f1(_logger, "CoCreateInstance(IMMDeviceEnumerator) failed: result = 0x%08?x", result);
    }

    return enumerator;
}

HRESULT AudioCaptureImpl::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    auto deviceId{UnicodeToString(pwstrDeviceId)};

    poco_trace_f2(_logger, "Audio device state changed for device ID %s: %lu", deviceId, dwNewState);

    // Recalculate current device index and restart only if not default.
    if (_currentAudioDeviceIndex >= 0)
    {
        IMMDeviceEnumerator* enumerator{GetDeviceEnumerator()};
        auto captureDevices{GetAudioDeviceList(enumerator)};
        enumerator->Release();

        for (int index = 0; index < captureDevices.size(); ++index)
        {
            if (UnicodeToString(captureDevices.at(index).DeviceId()) == deviceId)
            {
                _currentAudioDeviceIndex = index;
                break;
            }
        }

        // Restart capturing only in case the current device state changed.
        if (_isCapturing && deviceId == _currentCaptureDeviceId)
        {
            _restartCapturing = true;
        }
    }

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
{
    poco_trace_f1(_logger, "Default audio device changed to ID %s", UnicodeToString(pwstrDefaultDeviceId));

    if (flow != eRender || _currentAudioDeviceIndex != -1)
    {
        return S_OK;
    }

    if (_isCapturing)
    {
        _restartCapturing = true;
    }

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    poco_trace_f1(_logger, "Audio device added: %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

HRESULT AudioCaptureImpl::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    poco_trace_f1(_logger, "Audio device removed: %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

HRESULT AudioCaptureImpl::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    poco_trace_f1(_logger, "Audio device property changed for device ID %s", UnicodeToString(pwstrDeviceId));

    return S_OK;
}

AudioCaptureImpl::AudioDevice::AudioDevice(IMMDevice* device, bool isRenderDevice)
    : _friendlyName(GetAudioEndpointFriendlyName(device))
    , _isRenderDevice(isRenderDevice)
{
    if (device != nullptr)
    {
        HRESULT result = device->GetId(&_deviceId);

        if (FAILED(result))
        {
            poco_trace_f1(_logger, "IMMDevice::GetId failed: result = 0x%08?x", result);
        }

        poco_trace_f3(_logger, R"(Added WASAPI audio device "%s" with ID %s (Render device: %b))",
                      _friendlyName, AudioCaptureImpl::UnicodeToString(_deviceId), _isRenderDevice);
    }
}

AudioCaptureImpl::AudioDevice::AudioDevice(AudioCaptureImpl::AudioDevice&& other) noexcept
{
    _deviceId = other._deviceId;
    other._deviceId = nullptr;
    _friendlyName = std::move(other._friendlyName);
    _isRenderDevice = other._isRenderDevice;
}

AudioCaptureImpl::AudioDevice::~AudioDevice()
{
    if (_deviceId)
    {
        CoTaskMemFree(_deviceId);
        _deviceId = nullptr;
    }
}

std::string AudioCaptureImpl::AudioDevice::GetAudioEndpointFriendlyName(IMMDevice* pMMDevice)
{
    HRESULT result{S_OK};

    if (pMMDevice == nullptr)
    {
        return AudioCaptureImpl::_defaultDeviceName;
    }

    IPropertyStore* deviceProps{nullptr};
    result = pMMDevice->OpenPropertyStore(STGM_READ, &deviceProps);
    if (FAILED(result) || deviceProps == nullptr)
    {
        poco_error_f1(_logger, "IMMDevice::OpenPropertyStore failed: result = 0x%08?x", result);
        return {};
    }

    PROPVARIANT variantName;
    PropVariantInit(&variantName);

    result = deviceProps->GetValue(PKEY_Device_FriendlyName, &variantName);
    if (FAILED(result))
    {
        deviceProps->Release();
        poco_error_f1(_logger, "IMMDevice::OpenPropertyStore failed: result = 0x%08?x", result);
        return {};
    }

    std::string deviceFriendlyName = AudioCaptureImpl::UnicodeToString(variantName.pwszVal);

    PropVariantClear(&variantName);
    deviceProps->Release();

    return deviceFriendlyName;
}

LPWSTR AudioCaptureImpl::AudioDevice::DeviceId() const
{
    return _deviceId;
}

std::string AudioCaptureImpl::AudioDevice::FriendlyName() const
{
    return _friendlyName;
}

bool AudioCaptureImpl::AudioDevice::IsRenderDevice() const
{
    return _isRenderDevice;
}
