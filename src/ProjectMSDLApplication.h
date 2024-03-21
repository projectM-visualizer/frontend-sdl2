#pragma once

#include <Poco/Util/Application.h>
#include <Poco/Util/MapConfiguration.h>
#include <Poco/Util/PropertyFileConfiguration.h>

class ProjectMSDLApplication : public Poco::Util::Application
{
public:
    ProjectMSDLApplication();

    const char* name() const override;

    /**
     * @brief Returns the instance of the projectMSDL application.
     * @return The instance of the projectMSDL application.
     */
    static ProjectMSDLApplication& instance();

    /**
     * @brief Returns the user configuration layer.
     * @return The configuration instance which stores the settings for the current user.
     */
    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> UserConfiguration();

    /**
     * @brief Returns the command line override map.
     * @return The properties file instance which stores the UI settings.
     */
    Poco::AutoPtr<Poco::Util::MapConfiguration> CommandLineConfiguration();

protected:
    void initialize(Application& self) override;

    void uninitialize() override;

    void defineOptions(Poco::Util::OptionSet& options) override;

    int main(const std::vector<std::string>& args) override;

    /**
     * @brief Display help and exit.
     * @param name Unused.
     * @param value Unused.
     */
    void DisplayHelp(const std::string& name, const std::string& value);

    void ListAudioDevices(const std::string& name, const std::string& value);

    Poco::AutoPtr<Poco::Util::PropertyFileConfiguration> _userConfiguration{
        new Poco::Util::PropertyFileConfiguration()}; //!< The current user's configuration, used to store/reset changes made in the UI's settings dialog.
    Poco::AutoPtr<Poco::Util::MapConfiguration> _commandLineOverrides{
        new Poco::Util::MapConfiguration()}; //!< Map configuration with overrides set by command line arguments.
};
