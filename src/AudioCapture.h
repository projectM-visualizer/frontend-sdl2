#pragma once

#include AUDIO_IMPL_INCLUDE

#include <Poco/Util/Subsystem.h>

#include <memory>

class AudioCapture : public  Poco::Util::Subsystem
{
public:
    const char* name() const override;

protected:
    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    std::unique_ptr<AudioCaptureImpl> _impl;
};


