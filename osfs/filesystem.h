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
	void cat(const std::string &fileName) const;
	void rename( const std::string& src, const std::string& dst );
	void createImage(const std::string &saveFile) const;
	void restoreImage(const std::string &saveFile);
	void mkdir(std::string newName);
	void rmdir(std::string directory);
	void rm(const std::string &filePath);
	void copy(const std::string &src, const std::string &dst);
	void cd(const std::string& directory);
	void append(const std::string &src, const std::string &dst);
	void chmod(int permission, const std::string &file);

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
				unsigned EmptyBlockCount;
			};
			char x[512];
		};
	};

	struct FileBlock
	{
		char Name[16];
		unsigned Access;
		unsigned FileSize;
		unsigned char PayloadBlocks[488];
	};

private:
	void _GetFileBlocks(const Tree *directory, std::map<unsigned char, FileBlock>& fileBlocks) const;
	void _GetFilesCWD(void);
	void _ListDirectory(const Tree *directory) const;
	const Tree* _DirectoryOf(const std::string& path) const;
	void _RemoveFile(unsigned char file);
	void _SplitFilePath( const std::string& filePath, Tree **dir, std::string& dirString, std::string& file ) const;
	void _WriteToFile( const std::string& filePath, const char *data, unsigned dataSize );

private:
    MemBlockDevice mMemblockDevice;
    // Here you can add your own data structures
    Tree _root;
	Tree* _cwd = nullptr;
	std::map<unsigned char, FileBlock> _cwdFiles;
};

#endif // FILESYSTEM_H
