#include "ProjectMWrapper.h"

const char* ProjectMWrapper::name() const
{
    return "ProjectM Wrapper";
}

void ProjectMWrapper::initialize(Poco::Util::Application& app)
{
    if (!_projectM)
    {
        projectm_create_settings(nullptr, PROJECTM_FLAG_NONE);
    }
}

void ProjectMWrapper::uninitialize()
{
    if (_projectM)
    {
        projectm_destroy(_projectM);
        _projectM = nullptr;
    }
}

projectm* ProjectMWrapper::ProjectM() const
{
    return _projectM;
}
