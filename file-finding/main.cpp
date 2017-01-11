#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <math.h>
#include <vector>
#include <string>

long long inodeNo = -1;
long long sizeMin = -1;
long long sizeMax = -1;
std::string filename = "*";
std::string path = ".";

void parseArgs(int, char **);
void traverseDir(std::string);

int main(int argc, char **argv) {
	parseArgs(argc, argv);
	traverseDir(path);
	return 0;
}

void parseArgs(int argc, char **argv) {
	std::vector<std::string> args;
	for (int i = 0; i < argc; ++i)
		args.push_back(std::string(argv[i]));

	if (args.size() >= 2)
		path = args[1];

	auto it = std::find(args.begin(), args.end(), "-inode");
	if (it != args.end())
		if (++it != args.end())
			inodeNo = std::stoull(*it);

	it = std::find(args.begin(), args.end(), "-size_min");
	if (it != args.end())
		if (++it != args.end())
			sizeMin = std::stoull(*it);

	it = std::find(args.begin(), args.end(), "-size_max");
	if (it != args.end())
		if (++it != args.end())
			sizeMax = std::stoull(*it);

	it = std::find(args.begin(), args.end(), "-name");
	if (it != args.end())
		if (++it != args.end())
			filename = *it;
}

void traverseDir(std::string currPath) {
	DIR *dp;
	struct dirent *de;
	struct stat st;
	if ((dp = opendir(currPath.c_str())) == NULL)
		return;

	while ((de = readdir(dp))) {
		if (de->d_type == DT_DIR) {
			if (strcmp(de->d_name, ".") && strcmp(de->d_name, ".."))
				traverseDir(currPath + "/" + de->d_name);
		}
		else {
			bool passed = true;
			std::string filepath = currPath + "/" + de->d_name;
			stat(filepath.c_str(), &st);
			if (inodeNo != -1 && de->d_ino != inodeNo)
				passed = false;
			if (filename != "*" && de->d_name != filename)
				passed = false;
			if (sizeMin != -1 && st.st_size < sizeMin * std::pow(2.0, 20.0))
				passed = false;
			if (sizeMax != -1 && st.st_size > sizeMax * std::pow(2.0, 20.0))
				passed = false;
			if (passed) {
				std::cout << filepath << " " << de->d_ino << " ";
				std::cout << std::fixed << std::setprecision(6);
				std::cout << st.st_size / std::pow(2.0, 20.0) << " MB" << std::endl;
			}
		}
	}

	closedir(dp);
}
