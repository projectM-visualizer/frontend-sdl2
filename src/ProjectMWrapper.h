#pragma once

#include <libprojectM/projectM.h>

#include <Poco/Logger.h>

#include <Poco/Util/Subsystem.h>
#include <Poco/Util/AbstractConfiguration.h>

#include <memory>

class ProjectMWrapper : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    /**
     * Returns the projectM instance handle.
     * @return The projectM instance handle used to call API functions.
     */
    projectm* ProjectM() const;

    /**
     * Renders a single projectM frame.
     */
    void RenderFrame() const;

    int TargetFPS();

    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;


protected:
    /**
     * @brief Sets the text of the help menu overlay.
     */
    void SetHelpText();

    Poco::AutoPtr<Poco::Util::AbstractConfiguration> _config; //!< View of the "projectM" configuration subkey.

    projectm* _projectM{ nullptr }; //!< Pointer to the projectM instance used by the application.

    Poco::Logger& _logger{ Poco::Logger::get("SDLRenderingWindow") }; //!< The class logger.
};


