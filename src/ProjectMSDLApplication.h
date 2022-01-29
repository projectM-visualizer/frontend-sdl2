#pragma once

#include <Poco/Util/Application.h>

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
};


