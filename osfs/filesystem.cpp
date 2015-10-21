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

	// Master and index of empty blocks.
	const unsigned numHardCodedBlocks = 2;

	// Create the master block
	MasterBlock masterBlock;
	masterBlock.EmptyBlockCount = 250 - numHardCodedBlocks;
	mMemblockDevice.writeBlock(0, reinterpret_cast<char*>(&masterBlock));

	//// Empty block initialization.
	//// Have the empty blocks point to the next.
	//for (unsigned i = 1; i < 250; ++i)
	//{
	//	char data[512];
	//	unsigned nextEmptyBlock = i + 1;
	//	if (i == 249) nextEmptyBlock = 0;
	//	memcpy(data, &nextEmptyBlock, sizeof(unsigned));
	//	mMemblockDevice.writeBlock(i, data);
	//}

	// Write the empty block indices.
	char tempBuffer[512];
	for (char i = 0; i < masterBlock.EmptyBlockCount; ++i)
	{
		// All blocks except the first hard coded ones are empty.
		tempBuffer[i] = i + numHardCodedBlocks;
	}

	// Zero rest of memory
	memset(tempBuffer + masterBlock.EmptyBlockCount, 0, 512 - masterBlock.EmptyBlockCount);

	mMemblockDevice.writeBlock(1, tempBuffer);

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

	while (cpy.length())
	{
		int firstSlash = cpy.find_first_of(delim);

		if (firstSlash == 0)
		{
			cpy.erase(0, 1);
			continue;
		}

		if (firstSlash == -1)
		{
			output.push_back(cpy);
			break;
		}

		output.push_back(cpy.substr(0, firstSlash));
		cpy = cpy.substr(firstSlash + 1);
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
		file = filePath.substr(index + 1);

		if (tempTree == NULL)
		{
			mkdir(dir);
			tempTree = const_cast<Tree*>(_DirectoryOf(dir));
		}

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
	for (int i = 0;i < 488;i++)
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

void FileSystem::rmdir(string directory)
{
	Tree *d = const_cast<Tree*>(_DirectoryOf(directory));
	Tree *p = d ? d->Parent() : nullptr;
	if (p)
	{
		string remove = *d - *p;
		p->RemoveSubdirectory(remove, [this](int file) {
			_RemoveFile(file);
		});
	}
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

// TODO: Don't forget to remove payloads!
void FileSystem::_RemoveFile(int file)
{
	//MasterBlock mb;
	//Block b = mMemblockDevice.readBlock(0);
	//memcpy(&mb, b.data(), 512);

	//unsigned firstEmptyBeforeFile = 0;
	//unsigned walker = mb.FirstEmptyBlock;
	//
	//while (walker < file && walker > 0)
	//{
	//	firstEmptyBeforeFile = walker;

	//	// Go to next empty block. Just copy the next block pointer
	//	// into the walker variable.
	//	b = mMemblockDevice.readBlock(walker);
	//	memcpy(&walker, b.data(), 4);
	//}

	//// The file block
	//b = mMemblockDevice.readBlock(file);

	//// No empty block before file
	//if (firstEmptyBeforeFile == 0)
	//{
	//	// Since we didn't find a first empty block before the file, mb.FirstEmptyBlock
	//	// is the next empty block after the file. The file we are removing should
	//	// therefore point to that one.
	//	char tempBuffer[512];
	//	memcpy(tempBuffer, &mb.FirstEmptyBlock, 4);
	//	mMemblockDevice.writeBlock(file, tempBuffer);

	//	// The file block we are removing will be new first empty block.
	//	mb.FirstEmptyBlock = file;
	//	mMemblockDevice.writeBlock(0, (char*)&mb);
	//}
	//// Found first empty block before file from walking
	//else
	//{
	//	// Link empty block before to file
	//	char tempBuffer[512];
	//	memcpy(tempBuffer, &file, 4);
	//	mMemblockDevice.writeBlock(firstEmptyBeforeFile, tempBuffer);

	//	// Link file to walker (walker is now the first empty after)
	//	memcpy(tempBuffer, &walker, 4);
	//	mMemblockDevice.writeBlock(file, tempBuffer);
	//}
}

// [KLART]
// format
// create <filnamn>
// ls
// mkdir <katalog>
// cd <katalog>
// hantera absoluta och relativa sökvägar

// [KVAR]
// createImage <filnamn> (spara systemet på datorns hårddisk)
// restoreImage <filnamn>
// cat <filnamn> skriv ut innehåll på skärm
// copy <fil1> <fil2> skapa ny fil fil2 som är kopia av fil1 (glöm ej; fungera från en mapp till en annan)
// pwd
// rm <filnamn> tar bort en given fil
// append <fil1> <fil2> lägger till innehåll från fil1 i slutet av fil2
// rename <fil1> <fil2> ändrar namn på fil fil1 till fil2
// katalognamn . och ..
// chmod <access> <filnamn> (dokumentera vilka koder som gör vad)
// cat med access
// write med access

// Vad vi gjort annorlunda:
// Lagt till en funktion på Block som returnerar data
// Tree