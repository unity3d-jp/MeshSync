#pragma once
namespace ms {
class msErrorHandler
{
    std::string m_export_errors;

public:
    std::string getExportErrors() const;
    void addExportError(std::string error);
    void clearExportErrors();
};
}