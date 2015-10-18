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

	cd("/");

	_cwd = &_root;
	_cwd->AddSubdirectory("first");
	_cwd->AddSubdirectory("second");
	_cwd->AddFile(1);
	_cwd = _cwd->GetDirectory("first");
	_cwd->AddFile(2);
	_cwd->AddFile(3);
	_cwd = &_root;
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

	Tree *oldWD = _cwd;

	// Begins with '/': absoute path, relative otherwise. In case of absolute
	// we want to start from the root instead of the current directory.
	if (directory[0] == '/')
		_cwd = &_root;

	// Attempt to navigate to every new subdirectory.
	for (unsigned i = 0; i < newPath.size(); ++i)
	{
		_cwd = _cwd->GetDirectory(newPath[i]);
		if (!_cwd)
			break;
	}

	// If _cwd is invalid it means that a certain subdirectory was not found.
	// We return to the one we were in before starting.
	if (!_cwd)
	{
		_cwd = oldWD;
		cout << "Could not find directory '" << directory << "'." << endl;
		return;
	}

	// If everything worked out properly, _cwd will now be the correct directory.
	_GetFilesCWD();
}

void FileSystem::_GetFilesCWD(void)
{
	_cwdFiles.clear();
	
	auto& files = _cwd->GetFiles();
	for_each(files.begin(), files.end(), [this](int file) {
		const void *blockData = mMemblockDevice.readBlock(file).data();
		FileBlock block;
		memcpy(&block, blockData, 512);
		_cwdFiles[file] = block;
	});
}