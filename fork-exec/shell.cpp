/**
 * @file shell.cpp - A Simple Shell
 * @brief A simple shell that supports background execution (&), redirection (<, >), and pipe(|).
 * @author Yu-wen Pwu (0316213)
 */

#include <algorithm>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

int main() {
	setenv("TERM", "xterm-256color", 0);
	// prevent "WARNING: terminal is not fully functional"
	// or "TERM environment variable not set" messages

	while (1) {
		std::cout << "> " << std::flush;
		std::string command;
		std::getline(std::cin, command);
		std::istringstream ss(command);

		std::vector<std::string> arguments;
		std::string argument;
		while (ss >> argument)
			arguments.push_back(argument);

		if (arguments.front() == "exit")
			return 0;
		bool blocking = arguments.back() != "&";
		if (arguments.back() == "&")
			arguments.pop_back();

		bool redirect_in = false, redirect_out = false;
		std::string redirect_in_path = "", redirect_out_path = "";
		std::vector<std::string>::iterator it_find;

		it_find = std::find(arguments.begin(), arguments.end(), "<");
		if (it_find != arguments.end()) {
			redirect_in = true;
			redirect_in_path = *(it_find + 1);
			arguments.erase(it_find, arguments.end());
		}

		it_find = std::find(arguments.begin(), arguments.end(), ">");
		if (it_find != arguments.end()) {
			redirect_out = true;
			redirect_out_path = *(it_find + 1);
			arguments.erase(it_find, arguments.end());
		}

		char **c_arguments = new char*[arguments.size() + 1];
		int i = 0;
		for (std::vector<std::string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
			char *c_argument = new char[it->size() + 1];
			std::copy(it->begin(), it->end(), c_argument);
			c_argument[it->size()] = '\0';
			c_arguments[i++] = c_argument;
		}
		c_arguments[i++] = NULL;

		if (blocking)
			signal(SIGCHLD, SIG_DFL);
		else
			signal(SIGCHLD, SIG_IGN);
		// find pid of shell: ps aux | grep shell
		// find all defunct processes: ps aux | grep Z
		// find child processes of pid: pstree -p [pid]

		pid_t pid = fork();
		if (pid < 0)
			std::cerr << "fork() error" << std::endl;

		else if (pid == 0) {
			// child process
			if (redirect_in) {
				int fin = open(redirect_in_path.c_str(), O_RDONLY);
				dup2(fin, STDIN_FILENO);
				close(fin);
			}
			// open input file
			if (redirect_out) {
				int fout = open(
					redirect_out_path.c_str(),
					O_WRONLY | O_CREAT,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
				);
				dup2(fout, STDOUT_FILENO);
				close(fout);
			}
			// open or create output file in mode 644
			execvp(c_arguments[0], c_arguments);
			// the exec family: l for list, v for vector, p for path, e for environment
			_exit(1);
			// error occurred
		}

		else {
			if (blocking) {
				int status;
				waitpid(pid, &status, 0);
				usleep(100);
				// workaround for overlapped output
				if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0))
					std::cout << "shell: command execution failed: " << command << std::endl;
			}

			for (int j = 0; j < i; ++j)
				delete[] c_arguments[j];
			delete[] c_arguments;
		}
	}

}

// TODO:
// http://stackoverflow.com/questions/17166721/pipe-implementation-in-linux-using-c
// http://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell
