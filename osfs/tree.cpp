#include "tree.h"

using namespace std;

void Tree::AddSubdirectory(string subDirectory)
{
	if (_subDirectories.find(subDirectory) == _subDirectories.end())
	{
		_subDirectories[subDirectory] = Tree((_path == "/" ? "/" : _path) + subDirectory, this);
	}
}

void Tree::_Remove(function<void(int)> RemoveFile)
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

}

void Tree::RemoveFile(int file)
{
	for (int i = 0;i < _files.size();i++)
	{
		if (file == _files[i])
		{
			_files.erase(_files.begin()+i);
			break;
		}
	}
}

void Tree::RemoveSubdirectory(string subDirectory, function<void(int)> RemoveFile)
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

void Tree::AddFile(int file)
{
	_files.push_back(file);
}

const vector<int>& Tree::GetFiles(void) const
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

	return _path.substr(rhs._path.length());
}