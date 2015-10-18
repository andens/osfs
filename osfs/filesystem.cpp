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
	//Kolla ifall filen redan finns
	for (auto i = _cwdFiles.begin();i != _cwdFiles.end();i++)
	{
		if(i->second.Name == filePath)
		{
			cout << "File already finns" << endl;
			return;
		}
	}

	//Creating new FileBlock

	FileBlock newFile;

	if(filePath.size()<16)
	{
		cout << "Name is too big" << endl;
	}
	
	strcpy(newFile.Name, filePath.c_str());
	newFile.Access = 0;
	newFile.FileSize = 0;
	for (int i = 0;i < 122;i++)
	{
		newFile.PayloadBlocks[i] = 0;
	}

	//step two leta reda på ledig plats
	Block temp;
	//fetching master block
	temp = mMemblockDevice.readBlock(0);

	MasterBlock masterTemp;
	//creating a masterBlock
	memcpy(&masterTemp, temp.data(), 512);
	unsigned BlockSlott = masterTemp.FirstEmptyBlock;
	//fetching second Empty block
	Block _temp = mMemblockDevice.readBlock(masterTemp.FirstEmptyBlock);
	// Copy next empty block pointer
	memcpy(&masterTemp.FirstEmptyBlock, _temp.data(), 4); 
	mMemblockDevice.writeBlock(0, (char*)&masterTemp);
	//step tre skriva till sagda plats
	mMemblockDevice.writeBlock(BlockSlott, (char*)&newFile);
	//lägg till fil i cwd
	_cwd->AddFile(BlockSlott);


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

void FileSystem::_GetFilesCWD(void)
{

}