#include "pch.h"
#include "Test.h"
#include "MeshGenerator.h"

static std::string g_log;

void PrintImpl(const char *format, ...)
{
    const int MaxBuf = 4096;
    char buf[MaxBuf];

    va_list args;
    va_start(args, format);
    vsprintf(buf, format, args);
    g_log += buf;
#ifdef _WIN32
    ::OutputDebugStringA(buf);
#endif
    printf("%s", buf);
    va_end(args);
    fflush(stdout);
}

struct ArgEntry
{
    std::string name;
    std::string value;

    template<class T> bool getValue(T& dst) const;
};
template<> bool ArgEntry::getValue(std::string& dst) const
{
    dst = value;
    return true;
}
template<> bool ArgEntry::getValue(int& dst) const
{
    dst = std::atoi(value.c_str());
    return dst != 0;
}
template<> bool ArgEntry::getValue(float& dst) const
{
    dst = (float)std::atof(value.c_str());
    return dst != 0.0f;
}

static std::vector<ArgEntry>& GetArgs()
{
    static std::vector<ArgEntry> s_instance;
    return s_instance;
}
template<class T> bool GetArg(const char *name, T& dst)
{
    auto& args = GetArgs();
    auto it = std::find_if(args.begin(), args.end(), [name](auto& e) { return e.name == name; });
    if (it != args.end())
        return it->getValue(dst);
    return false;
}
template bool GetArg(const char *name, std::string& dst);
template bool GetArg(const char *name, int& dst);
template bool GetArg(const char *name, float& dst);



struct TestEntry
{
    std::string name;
    std::function<void()> body;
};

static std::vector<TestEntry>& GetTests()
{
    static std::vector<TestEntry> s_instance;
    return s_instance;
}

void RegisterTestEntryImpl(const char *name, const std::function<void()>& body)
{
    GetTests().push_back({name, body});
}

static void RunTestImpl(const TestEntry& v)
{
    Print("%s begin\n", v.name.c_str());
    auto begin = Now();
    v.body();
    auto end = Now();
    Print("%s end (%.2fms)\n\n", v.name.c_str(), NS2MS(end-begin));
}

testExport const char* GetLogMessage()
{
    return g_log.c_str();
}

testExport void RunTest(char *name)
{
    g_log.clear();
    for (auto& entry : GetTests()) {
        if (entry.name == name) {
            RunTestImpl(entry);
        }
    }
}

testExport void RunAllTests()
{
    g_log.clear();
    for (auto& entry : GetTests()) {
        RunTestImpl(entry);
    }
}

int main(int argc, char *argv[])
{
    int run_count = 0;
    for (int i = 1; i < argc; ++i) {
        if (char *sep = std::strstr(argv[i], "=")) {
            *(sep++) = '\0';
            GetArgs().push_back({ argv[i] , sep });
        }
        else {
            RunTest(argv[i]);
            ++run_count;
        }
    }
    if (run_count == 0) {
        RunAllTests();
    }
}
