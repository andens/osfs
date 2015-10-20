#ifndef TREE_H_
#define TREE_H_

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <algorithm>

class Tree
{
public:
	Tree() : _path("/") {};
	Tree(std::string path) : _path(path) {};
	void AddSubdirectory(std::string subDirectory);
	void RemoveSubdirectory(std::string subDirectory, std::function<void(int)> RemoveFile);
	void AddFile(int file);
	const Tree* GetDirectory(std::string subDirectory) const;
	const std::vector<int>& GetFiles(void) const;
	std::vector<std::string> GetSubdirectories(void) const;
	void RemoveFile(int file);
	std::string GetPath(void) const { return _path; }

private:
	void _Remove(std::function<void(int)> RemoveFile);

private:
	std::string _path;
	std::vector<int> _files;
	std::map<std::string, Tree> _subDirectories;
};

#endif