#include "filesystem.h"

#include <iostream>
#include <string.h>
#include <algorithm>

using namespace std;

FileSystem::FileSystem() : _cwd(&_root) {
	format();
}

void FileSystem::format(void)
{
	mMemblockDevice.reset();

	// Create the master block
	MasterBlock masterBlock;
	masterBlock.FirstEmptyBlock = 2;
	mMemblockDevice.writeBlock(0, reinterpret_cast<char*>(&masterBlock));

	// Create the root directory
	// DirectoryBlock rootDirectory;
	// strcpy(rootDirectory.Name, "");
	// rootDirectory.ParentDirectory = 0;
	// rootDirectory.NextDirectoryBlock = 0;
	// rootDirectory.ChildDirectoryCount = 0;
	// rootDirectory.FileCount = 0;
	// mMemblockDevice.writeBlock(1, reinterpret_cast<char*>(&rootDirectory));

	// Empty block initialization.
	// Have the empty blocks point to the next.
	for (unsigned i = 2; i < 250; ++i)
	{
		char data[512];
		unsigned nextEmptyBlock = i + 1;
		if (i == 249) nextEmptyBlock = 0;
		memcpy(data, &nextEmptyBlock, sizeof(unsigned));
		mMemblockDevice.writeBlock(i, data);
	}

	_cwd = &_root;
	_cwd->AddSubdirectory("first");
	_cwd->AddSubdirectory("second");
	_cwd->AddFile(1);
	_cwd = _cwd->GetDirectory("first");
	_cwd->AddFile(2);
	_cwd->AddFile(3);
	_cwd = &_root;

	//_cwdBlock = mMemblockDevice.readBlock(1);
}

vector<string> FileSystem::_Split(const string &filePath, const char delim) const {
    vector<string> output;
    string cpy = filePath;

    size_t end = cpy.find_last_of(delim);
    if (cpy.length() > end+1) {
        output.push_back(cpy.substr(end+1, cpy.length()));
    }

    while (end != 0 && end!= std::string::npos) {

        cpy = cpy.substr(0, cpy.find_last_of('/'));
        //cout << cpy << endl;
        end = cpy.find_last_of(delim);
        output.push_back(cpy.substr(end+1, cpy.length()));

    }

    return output;
}

void FileSystem::ls(void) const
{
	const vector<int>& files = _cwd->GetFiles();
	vector<string> subDirs = _cwd->GetSubdirectories();

	for_each(files.begin(), files.end(), [](int file) {
		cout << "File: " << file << endl;
	});

	for_each(subDirs.begin(), subDirs.end(), [](string dir) {
		cout << "Subdir: " << dir << endl;
	});
}

void FileSystem::create(const std::string &filePath)
{
	const vector<int>& files = _cwd->GetFiles();

	for (int i = 0;i < files.size();i++)
	{

	}

}

void FileSystem::mkdir(std::string newName)
{
	_cwd->AddSubdirectory(newName);

}

void FileSystem::rm(const std::string &filePath)
{
	const vector<int>& files = _cwd->GetFiles();

	for (int i = 0;i < files.size();i++)
	{

	}
}

void FileSystem::cd(const string& directory)
{
	if (directory.length() == 0)
		return;

	vector<string> newPath = _Split(directory);

	// Begins with '/': absoute path, relative otherwise
	if (directory[0] == '/')
	{
		Tree *oldWD = _cwd;
		_cwd = &_root;
		for (unsigned i = 0; i < newPath.size(); ++i)
		{
			_cwd = _cwd->GetDirectory(newPath[i]);
			if (!_cwd)
				break;
		}

		if (!_cwd)
		{
			_cwd = oldWD;
			cout << "Could not find directory '" << directory << "'." << endl;
		}
	}
	else
	{
		Tree *oldWD = _cwd;
		for (unsigned i = 0; i < newPath.size(); ++i)
		{
			_cwd = _cwd->GetDirectory(newPath[i]);
			if (!_cwd)
				break;
		}

		if (!_cwd)
		{
			_cwd = oldWD;
			cout << "Could not find directory '" << directory << "'." << endl;
		}
	}
}

/*
Screw this. Vi lagrar katalogstruktur i ett träd i arbetsminnet.
Varje nod är en katalog som innehåller andra noder (underkataloger).
Varje nod har även en array av int som motsvarar filblocken på disk.
När vi avslutar sparar vi strukturen som vilken annan fil som helst,
fast vi kan ju hårdkoda filblocket till block med index 1.
*/
// void FileSystem::GetChildren(unsigned directoryBlock, Block **directories, Block **files) const
// {
// 	Block theDir = mMemblockDevice.readBlock(directoryBlock);
// 	unsigned directoryCount = reinterpret_cast<DirectoryBlock>(theDir).ChildDirectoryCount;
// 	unsigned fileCount = reinterpret_cast<DirectoryBlock>(theDir).FileCount;
// 	unsigned nextDirectoryBlock = reinterpret_cast<DirectoryBlock>(theDir).NextDirectoryBlock;

// 	if (directories)
// 	{
// 		*directories = new Block[directoryCount];
// 	}

// 	if (files)
// 	{
// 		*files = new Block[fileCount];
// 	}

// 	int byteOffset = 32; // Skip metadata for first block.
// 	unsigned totalChildCount = directoryCount + fileCount;
// 	unsigned childrenTraversed = 0;
// 	for (unsigned i = 0; i < totalChildCount; ++i)
// 	{
// 		unsigned index;
// 		memcpy(&index, (char*)&theDir + byteOffset, sizeof(unsigned));

// 		if (childrenTraversed < directoryCount) // We are on a directory
// 		{
// 			if (directories)
// 				*directories[childrenTraversed] = mMemblockDevice.readBlock(index);
// 		}
// 		else // We are on a file
// 		{
// 			if (files)
// 				*files[childrenTraversed - directoryCount] = mMemblockDevice.readBlock(index);
// 		}

// 		childrenTraversed++;
// 		byteOffset += 4;

// 		if (byteOffset >= 512)
// 		{
// 			theDir = mMemblockDevice.readBlock(nextDirectoryBlock);
// 			byteOffset = 4; // New block, skip pointer to next block.
// 			memcpy(&nextDirectoryBlock, (char*)&theDir, sizeof(unsigned)); // Get the pointer to the next block
// 		}
// 	}
// }