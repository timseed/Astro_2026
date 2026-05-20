#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#include <sys/stat.h>

#include "Options.h"
#include "xpDefines.h"
#include "xpUtil.h"

static bool
fileExists(string &filename)
{
    bool returnVal = false;

    ostringstream msg;
    msg << "Looking for " << filename << " ... ";
    ifstream f(filename.c_str());
    if (f.is_open())
    {
        struct stat status;
        stat(filename.c_str(), &status );
        if (status.st_mode & S_IFREG)
        {
            msg << "found\n";
            f.close();
            returnVal = true;
        }
        else
        {
            msg << "is not a regular file!\n";
            returnVal = false;
        }
    }
    else
    {
        msg << "not found\n";
        returnVal = false;
    }

    Options *options = Options::getInstance();
    if (options->Verbosity() > 2)
    {
        // only say we're looking for a file once
        static vector<string> searchedFor;
        bool firstTime = true;
        for (int i = searchedFor.size()-1; i >= 0; i--)
        {
            if (searchedFor[i] == filename)
            {
                firstTime = false;
                break;
            }
        }
        if (firstTime)
        {
            searchedFor.push_back(filename);
            xpMsg(msg.str(), __FILE__, __LINE__);
        }
    }

    return(returnVal);
}

bool
findFile(string &filename, const string &subdir)
{
    // Check if the file exists in the current directory before going
    // to searchdir
    if (fileExists(filename)) return(true);

    Options *options = Options::getInstance();
    vector<string> searchdir = options->getSearchDir();

    string newname;
    for (int i = searchdir.size() - 1; i >= 0; i--)
    {
        // Check in searchdir itself
        newname = searchdir[i];
        newname += separator;
        newname += filename;

        if (fileExists(newname))
        {
            filename = newname;
            return(true);
        }

        // Now look in searchdir + subdir
        newname = searchdir[i];
        newname += separator;
        if (!subdir.empty())
        {
            newname += subdir;
            newname += separator;
        }
        newname += filename;

        if (fileExists(newname))
        {
            filename = newname;
            return(true);
        }
    }

    string errMsg("Can't find ");
    errMsg += filename;
    errMsg += " in\n";
    for (int i = searchdir.size() - 1; i >= 0; i--)
    {
        errMsg += searchdir[i];
        errMsg += separator;
        errMsg += subdir;
        errMsg += "\n";
    }
    xpWarn(errMsg, __FILE__, __LINE__);
    return(false);
}
