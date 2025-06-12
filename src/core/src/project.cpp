#include "core/project.h"

Project::Project(const ProjectInfo &info, QObject *parent)
    : QObject(parent)
    , m_info(info)
{
}
