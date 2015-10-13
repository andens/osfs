#ifndef TREE_H_
#define TREE_H_

#include <string>
#include <map>
#include <vector>
#include <functional>

class Tree
{
public:
	Tree();
	Tree(std::string name) : _name(name) {}
	void AddSubdirectory(std::string subDirectory);
	void RemoveSubdirectory(std::string subDirectory, std::function<void(int)> RemoveFile);
	void AddFile(int file);
	Tree& GetDirectory(std::string subDirectory); 

private:
	void _Remove(std::function<void(int)> RemoveFile);

private:
	std::string _name;
	std::vector<int> _files;
	std::map<std::string, Tree> _subDirectories;
};

#endif