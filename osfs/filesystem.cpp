#include "filesystem.h"

#include <iostream>
#include <string.h>
#include <algorithm>
#include <Windows.h>

using namespace std;

FileSystem::FileSystem() : _cwd(&_root) {
	format();
}

void FileSystem::format(void)
{
	mMemblockDevice.reset();

	// Create the master block
	MasterBlock masterBlock;
	masterBlock.FirstEmptyBlock = 1;
	mMemblockDevice.writeBlock(0, reinterpret_cast<char*>(&masterBlock));

	// Empty block initialization.
	// Have the empty blocks point to the next.
	for (unsigned i = 1; i < 250; ++i)
	{
		char data[512];
		unsigned nextEmptyBlock = i + 1;
		if (i == 249) nextEmptyBlock = 0;
		memcpy(data, &nextEmptyBlock, sizeof(unsigned));
		mMemblockDevice.writeBlock(i, data);
	}

	cd("/");
	_cwd->AddSubdirectory("first");
	_cwd->AddSubdirectory("second");
	create("a");
	cd("first");
	create("b");
	create("c");
	cd("/");
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
	_ListDirectory(_cwd);
}

void FileSystem::ls(const string &path) const
{
	_ListDirectory(_DirectoryOf(path));
}

void FileSystem::_ListDirectory(const Tree *directory) const
{
	map<int, FileBlock> fileBlocks;
	_GetFileBlocks(directory, fileBlocks);
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 259);

	vector<string> subDirs = directory->GetSubdirectories();
	for_each(subDirs.begin(), subDirs.end(), [](string dir) {
		cout << dir << "/" << endl;
	});

	SetConsoleTextAttribute(hConsole, 15);

	for_each(fileBlocks.begin(), fileBlocks.end(), [](const pair<int, FileBlock>& file) {
		cout << file.second.Name << " (" << file.second.FileSize << " bytes)" << endl;
	});
}

void FileSystem::create(const std::string &filePath)
{
	//Kolla ifall filen redan finns
	string file, dir;
	Tree* tempTree = const_cast<Tree*>(_DirectoryOf(dir));
	map<int, FileBlock> tempFileBlock;

	if (strcmp(&filePath[filePath.length() - 1], "/") == 0)
	{
		cout << "Path cannot end with a directory separator" << endl;
		return;
	}

	int index = filePath.find_last_of("/");
	if (index == -1)
	{
		// Just filename, create file at _cwd
		file = filePath;
		tempTree = _cwd;
		tempFileBlock = _cwdFiles;
	}
	else
	{
		dir = filePath.substr(0, index);
		cout << "Dir: " << dir << endl;
		file = filePath.substr(index + 1);
		cout << "File: " << file << endl;

	
	if (tempTree == NULL)
	{
		mkdir(dir);
		tempTree = const_cast<Tree*>(_DirectoryOf(dir));
	}
	ls();
		_GetFileBlocks(tempTree, tempFileBlock);

	}
	//Creating new FileBlock
	for (auto i = tempFileBlock.begin();i != tempFileBlock.end();i++)
	{
		if(i->second.Name == file)
		{
			cout << "File does already exists" << endl;
			return;
		}
	}

	FileBlock newFile;
	if(file.size()>15)
	{
		cout << "Name is too big" << endl;
		return;
	}

	strncpy_s(newFile.Name,file.c_str(),file.size());
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

	//lägg till fil i directory
	tempTree->AddFile(BlockSlott);

}

void FileSystem::mkdir(std::string newName)
{
	
	Tree* _temp = _cwd;

	vector<string> path = _Split(newName);
	
	if (newName[0] == '/')
	{
		_cwd = &_root; //Kek
	}
	
	for (int i = 0; i < path.size(); i++)
	{
		_cwd->AddSubdirectory(path[i]);
		cd(path[i]);
	}
	_cwd = _temp;
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
	const Tree *newDir = _DirectoryOf(directory);
	if (newDir)
		_cwd = const_cast<Tree*>(newDir);

	// If everything worked out properly, _cwd will now be the correct directory.
	_GetFilesCWD();
}

void FileSystem::_GetFilesCWD(void)
{
	_GetFileBlocks(_cwd, _cwdFiles);
}

void FileSystem::_GetFileBlocks(const Tree *directory, map<int, FileBlock>& fileBlocks) const
{
	fileBlocks.clear();

	auto& files = directory->GetFiles();

	for_each(files.begin(), files.end(), [this, &fileBlocks](int file) {
		Block blockData = mMemblockDevice.readBlock(file);
		FileBlock block;
		memcpy(&block, blockData.data(), 512);
		fileBlocks[file] = block;
	});
}

const Tree* FileSystem::_DirectoryOf(const string& path) const
{
	if (path.length() == 0)
		return nullptr;

	vector<string> newPath = _Split(path);

	const Tree *walker = _cwd;

	// Begins with '/': absoute path, relative otherwise. In case of absolute
	// we want to start from the root instead of the current directory.
	if (path[0] == '/')
		walker = &_root;

	// Attempt to navigate to every new subdirectory.
	for (unsigned i = 0; i < newPath.size(); ++i)
	{
		walker = walker->GetDirectory(newPath[i]);
		if (!walker)
			break;
	}

	// If walker is invalid it means that a certain subdirectory was not found.
	// We return to the one we were in before starting.
	if (!walker)
	{
		cout << "Could not find directory '" << path << "'." << endl;
		return nullptr;
	}

	return walker;
}