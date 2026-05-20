#ifndef FINDFILE_H
#define FINDFILE_H

#include <string>
#include <vector>

extern void
buildDirectoryVector(std::vector<std::string> &directoryVector, 
                     const std::string &subdir);

extern bool
findFile(std::string &filename, const std::string &subdir);

#endif

