#include "CppSystem.h"

#include <stdlib.h>

using namespace std;

string CppSystem::ExcuteCommand(const string &cmd, int32_t &ret)
{
    FILE *fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
    {
        THROW("popen return NULL");
    }

    char buf[BUFSIZ];
    string result;
    int32_t bufSize;

    while (!feof(fp))
    {
        bufSize = fread(buf, 1, sizeof(buf), fp);
        if (bufSize < 0)
        {
            break;
        }

        result.append(buf, bufSize);
    }

    ret = pclose(fp);
    ret = WEXITSTATUS(ret);

    return result;
}
