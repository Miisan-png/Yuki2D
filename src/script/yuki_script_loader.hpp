#pragma once
#include <string>
namespace yuki {
class ScriptLoader {
public:
    ScriptLoader(const std::string& path);
    std::string load();
private:
    std::string path;
};
}
