#include "ProjectMSDLApplication.h"

#include "AudioCapture.h"
#include "ProjectMWrapper.h"
#include "SDLRenderingWindow.h"

#include <Poco/Environment.h>
#include <Poco/File.h>

ProjectMSDLApplication::ProjectMSDLApplication()
{
    // Note: order here is important, as subsystems are initialized in the same order.
    addSubsystem(new SDLRenderingWindow);
    addSubsystem(new ProjectMWrapper);
    addSubsystem(new AudioCapture);
}

const char* ProjectMSDLApplication::name() const
{
    return "projectMSDL";
}

void ProjectMSDLApplication::initialize(Poco::Util::Application& self)
{
    loadConfiguration(PRIO_DEFAULT);

    // Try to load user's custom configuration file on top.
    Poco::Path userConfigurationFile = Poco::Path::configHome() + "projectM" + Poco::Path::separator() + "projectMSDL.properties";
    if (Poco::File(userConfigurationFile).exists())
    {
        loadConfiguration(userConfigurationFile.toString(), -10);
    }

    Application::initialize(self);
}

void ProjectMSDLApplication::uninitialize()
{
    Application::uninitialize();
}

void ProjectMSDLApplication::defineOptions(Poco::Util::OptionSet& options)
{

}

int ProjectMSDLApplication::main(const std::vector<std::string>& args)
{
    return EXIT_SUCCESS;
}

POCO_APP_MAIN(ProjectMSDLApplication)
