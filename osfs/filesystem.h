#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include "memblockdevice.h"
#include "tree.h"

class FileSystem
{
public:
	FileSystem();

	/* These commands needs to implemented */
	/*
	* However, feel free to change the signatures, these are just examples.
	* Remember to remove 'const' if needed.
	*/

	void format(void);
	void ls() const;
	void ls(const std::string &path) const;  // optional
	void create(const std::string &filePath);
	//std::string cat(std::string &fileName) const;
	//std::string createImage(const std::string &saveFile) const;
	//std::string restoreImage(const std::string &saveFile) const;
	void mkdir(std::string newName);
	void rm(const std::string &filePath);
	//std::string copy(const std::string &source, const std::string &dest);
	void cd(const std::string& directory);

	/* Optional */
	//std::string append(const std::string &source, const std::string &app);
	//std::string rename(const std::string &source, const std::string &newName);
	//std::string chmod(int permission, const std::string &file);

	/* Add your own member-functions if needed */
	std::string GetCWD(void) const { return _cwd->GetPath(); }
	std::vector<std::string> _Split(const std::string &filePath, const char delim = '/') const;

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

private:
	void _GetFileBlocks(const Tree *directory, std::map<int, FileBlock>& fileBlocks) const;
	void _GetFilesCWD(void);
	void _ListDirectory(const Tree *directory) const;

private:
    MemBlockDevice mMemblockDevice;
    // Here you can add your own data structures
    Tree _root;
	Tree* _cwd = nullptr;
	std::map<int, FileBlock> _cwdFiles;
};

#endif // FILESYSTEM_H
