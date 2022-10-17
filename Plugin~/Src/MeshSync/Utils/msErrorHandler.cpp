#include "MeshSync/Utility/msErrorHandler.h"

std::string ms::msErrorHandler::getExportErrors() const
{
	return m_export_errors;
}

void ms::msErrorHandler::addExportError(std::string error)
{
    // Add new line if there are already errors:
    if (m_export_errors.length() > 0)
    {
        m_export_errors += '\n';
    }

    m_export_errors += "- " + error;
}

void ms::msErrorHandler::clearExportErrors()
{
    m_export_errors = "";
}
