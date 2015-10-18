#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include "memblockdevice.h"
#include "tree.h"

class FileSystem
{
private:
    MemBlockDevice mMemblockDevice;
    // Here you can add your own data structures
    //unsigned _cwdBlock;
    Block *_cwdDirectories = nullptr;
    Block *_cwdFiles = nullptr;
    Tree _root;
    Tree& _cwd;

private:
    struct MasterBlock
    {
        union
        {
            struct
            {
                unsigned FirstEmptyBlock;
            };
            char x[512];
        };
    };

    struct FileBlock
    {
        char Name[16];
        unsigned Access;
        unsigned FileSize;
        unsigned PayloadBlocks[122];
    };

    // struct DirectoryBlock
    // {
    //     char Name[16];
    //     unsigned ParentDirectory;
    //     unsigned NextDirectoryBlock;
    //     unsigned ChildDirectoryCount;
    //     unsigned FileCount;
    //     unsigned Children[120];
    // };

public:
    FileSystem();

    /* These commands needs to implemented */
    /*
     * However, feel free to change the signatures, these are just examples.
     * Remember to remove 'const' if needed.
     */
    void format(void);
    void ls() const;
    std::string ls(const std::string &path) const;  // optional
    std::string create(const std::string &filePath);
    std::string cat(std::string &fileName) const;
    std::string createImage(const std::string &saveFile) const;
    std::string restoreImage(const std::string &saveFile) const;
    std::string rm(const std::string &filePath);
    std::string copy(const std::string &source, const std::string &dest);
    void cd(const std::string& directory);

    /* Optional */
    std::string append(const std::string &source, const std::string &app);
    std::string rename(const std::string &source, const std::string &newName);
    std::string chmod(int permission, const std::string &file);

    /* Add your own member-functions if needed */
    //std::vector<std::string> split(const std::string &filePath, const char delim = '/') const;
    //void GetChildren(unsigned directoryBlock, Block **directories, Block **files) const;
};

#endif // FILESYSTEM_H
