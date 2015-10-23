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

	// Write the empty block indices.
	unsigned char tempBuffer[512];
	for (unsigned char i = 0; i < masterBlock.EmptyBlockCount; ++i)
	{
		// All blocks except the first hard coded ones are empty.
		tempBuffer[i] = i + numHardCodedBlocks;
	}

	// Zero rest of memory
	memset(tempBuffer + masterBlock.EmptyBlockCount, 0, 512 - masterBlock.EmptyBlockCount);

	mMemblockDevice.writeBlock(1, (char*)tempBuffer);

	cd("/");
	_cwd->AddSubdirectory("first");
	_cwd->AddSubdirectory("second");
	_cwd->AddSubdirectory("a");
	create("a");
	// TODO: Write data to a. Create an array containing a null terminator in the middle of it and write all that data to file. Then cat it (it should not stop at null terminator hopefully).
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
	map<unsigned char, FileBlock> fileBlocks;
	_GetFileBlocks(directory, fileBlocks);
	
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 259);

	vector<string> subDirs = directory->GetSubdirectories();
	for_each(subDirs.begin(), subDirs.end(), [](string dir) {
		cout << dir << "/" << endl;
	});

	SetConsoleTextAttribute(hConsole, 15);

	vector<pair<unsigned char, FileBlock>> files;
	files.insert(files.begin(), fileBlocks.begin(), fileBlocks.end());
	sort(files.begin(), files.end(), [](const pair<unsigned char, FileBlock>& a, const pair<unsigned char, FileBlock>& b) {
		return strcmp(a.second.Name, b.second.Name) < 0;
	});
	for_each(files.begin(), files.end(), [](const pair<unsigned char, FileBlock>& file) {
		cout << file.second.Name << " (" << file.second.FileSize << " bytes)" << endl;
	});
}

void FileSystem::create(const std::string &filePath)
{
	//Kolla ifall filen redan finns
	string file, dir;
	Tree* tempTree=NULL;
	map<unsigned char, FileBlock> tempFileBlock;

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
		mkdir(dir);
		tempTree = const_cast<Tree*>(_DirectoryOf(dir));
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
	Block temp,_temp;


	//Getting master block
	temp = mMemblockDevice.readBlock(0);
	MasterBlock masterTemp;
	memcpy(&masterTemp, temp.data(), 512);

	//getting emptyy block pointer block
	unsigned BlockSlott = masterTemp.EmptyBlockCount;
	//getting empty block
	_temp = mMemblockDevice.readBlock(1);
	//getting new pointer place
	unsigned char newBlock = _temp.data()[BlockSlott - 1];
	//Create new block
	mMemblockDevice.writeBlock(newBlock, (char*)&newFile);

	//Decrese emptyblockCount 
	masterTemp.EmptyBlockCount--;
	//Write the new EmptyBlockCount back to masterBlock
	mMemblockDevice.writeBlock(0, (char*)&masterTemp);
	//lägg till fil i directory
	tempTree->AddFile(newBlock);
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
		p->RemoveSubdirectory(remove, [this](unsigned char file) {
			_RemoveFile(file);
		});
	}
}

void FileSystem::rm(const string &filePath)
{
	string file = "";
	Tree *directory = nullptr;

	_SplitFilePath( filePath, &directory, file );

	// We have the directory and filename. Get files in directory, search their
	// respective blocks to find the one we want and remove it.
	const vector<unsigned char>& files = directory->GetFiles();
	for (unsigned char fileIndex : files)
	{
		Block f = mMemblockDevice.readBlock(fileIndex);
		if (file == f.data())
		{
			_RemoveFile(fileIndex);
			directory->RemoveFile(fileIndex);
			return;
		}
	}

	// If we reached here, the file could not be found.
	cout << "Could not find file " << filePath << endl;
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

void FileSystem::_GetFileBlocks(const Tree *directory, map<unsigned char, FileBlock>& fileBlocks) const
{
	fileBlocks.clear();

	auto& files = directory->GetFiles();

	for_each(files.begin(), files.end(), [this, &fileBlocks](unsigned char file) {
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

void FileSystem::_RemoveFile(unsigned char file)
{
	MasterBlock mb;
	Block master = mMemblockDevice.readBlock(0);
	memcpy(&mb, master.data(), 512);

	char emptyFilesData[512];
	Block emptyFiles = mMemblockDevice.readBlock(1);
	memcpy(emptyFilesData, emptyFiles.data(), 512);

	FileBlock fb;
	Block removeBlock = mMemblockDevice.readBlock(file);
	memcpy(&fb, removeBlock.data(), 512);

	// Set the file itself as empty
	memcpy(emptyFilesData + mb.EmptyBlockCount, &file, 1);
	mb.EmptyBlockCount++;

	// Set payload blocks as empty
	unsigned payloadBlockCount = fb.FileSize / 513 + 1;
	memcpy(emptyFilesData + mb.EmptyBlockCount, fb.PayloadBlocks, payloadBlockCount);
	mb.EmptyBlockCount += payloadBlockCount;

	// Write master block and empty blocks block.
	mMemblockDevice.writeBlock(0, (char*)&mb);
	mMemblockDevice.writeBlock(1, (char*)emptyFilesData);
}

void FileSystem::cat(const string& filePath) const
{
	string file = "";
	Tree *directory = nullptr;

	_SplitFilePath( filePath, &directory, file );

	// We have the directory and filename. Get files in directory, search their
	// respective blocks to find the one we want.
	const vector<unsigned char>& files = directory->GetFiles();
	for (unsigned char fileIndex : files)
	{
		Block f = mMemblockDevice.readBlock(fileIndex);
		if (file == f.data())
		{
			FileBlock fb;
			memcpy(&fb, f.data(), 512);

			if (fb.FileSize == 0)
				return;

			for (unsigned i = 0, bytesRead = 0; bytesRead < fb.FileSize; ++i, bytesRead += 512)
			{
				unsigned remainingBytes = fb.FileSize - bytesRead;
				if (remainingBytes > 512) remainingBytes = 512;

				// Output remainingBytes worth of data
				cout.write(mMemblockDevice.readBlock(fb.PayloadBlocks[i]).data(), remainingBytes);
			}

			cout << endl;

			return;
		}
	}

	// If we reached here, the file could not be found.
	cout << "Could not find file " << filePath << endl;
}

void FileSystem::rename( const string& src, const string& dst )
{
	if ( dst.length() > 15 )
	{
		cout << "Destination file name too long" << endl;
		return;
	}

	auto& files = _cwd->GetFiles();

	// First check that the new file does not already exist.
	for ( unsigned char fileIndex : files )
	{
		FileBlock fb;
		memcpy( &fb, mMemblockDevice.readBlock( fileIndex ).data(), 512 );

		if ( dst == fb.Name )
		{
			cout << "Destination file already exists" << endl;
			return;
		}
	}

	// We are free to rename. Try to find the file, and if found rename it.
	for ( unsigned char fileIndex : files )
	{
		FileBlock fb;
		memcpy( &fb, mMemblockDevice.readBlock( fileIndex ).data(), 512 );

		// Found it! Time to rename.
		if ( src == fb.Name )
		{
			strcpy_s( fb.Name, dst.c_str() );
			mMemblockDevice.writeBlock( fileIndex, (char*)&fb );

			return;
		}
	}

	// If we reached here it means we never returned in the previous loop,
	// and so the source file was not found.
	cout << "Source file not found" << endl;
}

void FileSystem::_SplitFilePath( const string& filePath, Tree **dir, string& file ) const
{
	int lastSlash = filePath.find_last_of( "/" );

	// Found no slash; filePath is the file relative to current directory.
	if ( lastSlash == -1 )
	{
		file = filePath;
		*dir = _cwd;
	}
	// Found slash; file is part after last slash, directory is the part before.
	else
	{
		file = filePath.substr( lastSlash + 1 );
		*dir = const_cast<Tree*>(_DirectoryOf( filePath.substr( 0, lastSlash ) ));
	}
}

// [KLART]
// format
// create <filnamn>
// ls
// mkdir <katalog>
// cd <katalog>
// hantera absoluta och relativa sökvägar
// rm <filnamn> tar bort en given fil
// pwd
// rename <fil1> <fil2> ändrar namn på fil fil1 till fil2

// [KVAR]
// createImage <filnamn> (spara systemet på datorns hårddisk)
// restoreImage <filnamn>
// cat <filnamn> skriv ut innehåll på skärm
// copy <fil1> <fil2> skapa ny fil fil2 som är kopia av fil1 (glöm ej; fungera från en mapp till en annan)
// append <fil1> <fil2> lägger till innehåll från fil1 i slutet av fil2
// katalognamn . och ..
// chmod <access> <filnamn> (dokumentera vilka koder som gör vad)
// cat med access
// write med access

// Vad vi gjort annorlunda:
// Lagt till en funktion på Block som returnerar data
// Tree