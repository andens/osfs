#include "tree.h"

using namespace std;

Tree* Tree::AddSubdirectory(string subDirectory)
{
	if (_subDirectories.find(subDirectory) == _subDirectories.end())
	{
		//_subDirectories[subDirectory] = Tree((_path == "/" ? "/" : _path) + subDirectory, this);
		_subDirectories[subDirectory] = Tree(_path + ((_path == "/") ? "" : "/") + subDirectory, this);
		return &_subDirectories[subDirectory];
	}

	return nullptr;
}

void Tree::RemoveFile(unsigned char file)
{
	for (unsigned i = 0; i < _files.size(); ++i)
	{
		if (_files[i] == file)
		{
			_files.erase(_files.begin() + i);
			return;
		}
	}
}

void Tree::_Remove(function<void(unsigned char)> RemoveFile)
{
	for (auto i = _subDirectories.begin(); i != _subDirectories.end(); ++i)
	{
		i->second._Remove(RemoveFile);
	}

	_subDirectories.erase(_subDirectories.begin(), _subDirectories.end());

	for (unsigned i = 0; i < _files.size(); ++i)
	{
		RemoveFile(_files[i]);
	}

	_files.clear();
}

void Tree::RemoveSubdirectory(string subDirectory, function<void(unsigned char)> RemoveFile)
{
	if (_subDirectories.find(subDirectory) == _subDirectories.end())
		return;

	_subDirectories[subDirectory]._Remove(RemoveFile);
	_subDirectories.erase(subDirectory);
}

const Tree* Tree::GetDirectory(string subDirectory) const
{
	auto it = _subDirectories.find(subDirectory);

	if (it == _subDirectories.end())
		return nullptr;

	return &it->second;
}

void Tree::AddFile(unsigned char file)
{
	_files.push_back(file);
}

const vector<unsigned char>& Tree::GetFiles(void) const
{
	return _files;
}

vector<string> Tree::GetSubdirectories(void) const
{
	vector<string> subDirs;
	for (auto i = _subDirectories.begin(); i != _subDirectories.end(); ++i)
		subDirs.push_back(i->first);

	return subDirs;
}

std::string Tree::operator-(const Tree& rhs)
{
	if (rhs._path.length() > _path.length())
		return "";

	if ( rhs._path.length() == 1 ) // root
		return _path.substr( rhs._path.length() ); // root does not have a separator (its name is '/')
	else
		return _path.substr( rhs._path.length() + 1 );
}