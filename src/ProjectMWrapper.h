#pragma once

#include <libprojectM/projectM.h>

#include <Poco/Util/Subsystem.h>

class ProjectMWrapper : public Poco::Util::Subsystem
{
public:
    const char* name() const override;

    /**
     * Returns the projectM instance handle.
     * @return The projectM instance handle used to call API functions.
     */
    projectm* ProjectM() const;

protected:
    void initialize(Poco::Util::Application& app) override;

    void uninitialize() override;

    projectm* _projectM{ nullptr };
};


