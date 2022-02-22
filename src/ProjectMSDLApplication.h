#pragma once

#include <Poco/Util/Application.h>
#include <Poco/Util/MapConfiguration.h>

class ProjectMSDLApplication : public Poco::Util::Application
{
public:
    ProjectMSDLApplication();

    const char* name() const override;

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

    Poco::AutoPtr<Poco::Util::MapConfiguration> _commandLineOverrides{
        new Poco::Util::MapConfiguration() }; //!< Map configuration with overrides set by command line arguments.
};


