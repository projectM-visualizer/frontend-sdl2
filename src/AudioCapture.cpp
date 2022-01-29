#include "AudioCapture.h"

const char* AudioCapture::name() const
{
    return "Audio Capturing";
}

void AudioCapture::initialize(Poco::Util::Application& app)
{
    _impl = std::make_unique<AudioCaptureImpl>();
}

void AudioCapture::uninitialize()
{
    _impl.reset();
}
