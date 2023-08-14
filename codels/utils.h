#include <cstdio>
#include <string>
#include <sstream>

/* --- Setup ------------------------------------------------------ */

std::string executeCommand(const std::string &command)
{
    std::ostringstream output;
    FILE *pipe = popen(command.c_str(), "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed.");
    }

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        output << buffer;
    }

    int status = pclose(pipe);
    if (status != 0)
    {
        throw std::runtime_error("Command execution failed.");
    }

    return output.str().substr(0, output.str().size() - 1);
}