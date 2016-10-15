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
	// prevent "WARNING: terminal is not fully functional"
	// or "TERM environment variable not set" messages
	setenv("TERM", "xterm-256color", 0);

	while (1) {
		// read command
		std::cout << "> " << std::flush;
		std::string command;
		std::getline(std::cin, command);
		std::istringstream ss(command);

		// tokenize command
		std::vector<std::string> arguments;
		std::string argument;
		while (ss >> argument)
			arguments.push_back(argument);

		// fundamental check
		if (arguments.size() == 0)
			continue;
		if (arguments.front() == "exit")
			return 0;

		// check background execution (&)
		bool blocking = arguments.back() != "&";
		if (arguments.back() == "&") {
			arguments.pop_back();
			if (arguments.size() == 0)
				continue;
		}

		// check redirection (<, >)
		bool redirect_in = false, redirect_out = false;
		std::string redirect_in_path = "", redirect_out_path = "";
		std::vector<std::string>::iterator it_find;

		it_find = std::find(arguments.begin(), arguments.end(), "<");
		if (it_find != arguments.end()) {
			if (it_find + 1 != arguments.end()) {
				redirect_in = true;
				redirect_in_path = *(it_find + 1);
			}
			arguments.erase(it_find, arguments.end());
			if (arguments.size() == 0)
				continue;
		}

		it_find = std::find(arguments.begin(), arguments.end(), ">");
		if (it_find != arguments.end()) {
			if (it_find + 1 != arguments.end()) {
				redirect_out = true;
				redirect_out_path = *(it_find + 1);
			}
			arguments.erase(it_find, arguments.end());
			if (arguments.size() == 0)
				continue;
		}

		// check pipe(|)
		bool need_pipe = false;
		int next_cmd_idx = -1, fpipe[2];

		if (arguments.front() == "|") {
			arguments.erase(arguments.begin());
			if (arguments.size() == 0)
				continue;
		}

		if (arguments.back() == "|") {
			arguments.pop_back();
			if (arguments.size() == 0)
				continue;
		}

		if (std::find(arguments.begin(), arguments.end(), "|") != arguments.end()) {
			pipe(fpipe);
			need_pipe = true;
		}

		// convert C++ string vector to C char* array
		char **c_arguments = new char*[arguments.size() + 1];
		int arg_len = 0;
		for (std::vector<std::string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
			if (*it == "|") {
				c_arguments[arg_len++] = NULL;
				next_cmd_idx = arg_len;
			}
			else {
				char *c_argument = new char[it->size() + 1];
				std::copy(it->begin(), it->end(), c_argument);
				c_argument[it->size()] = '\0';
				c_arguments[arg_len++] = c_argument;
			}
		}
		c_arguments[arg_len++] = NULL;

		// find pid of shell: ps aux | grep shell
		// find all defunct processes: ps aux | grep Z
		// find child processes of pid: pstree -p -s [pid]
		pid_t pid = fork();

		// handle error
		if (pid < 0) {
			std::cerr << "shell: fork() failed" << std::endl;
			continue;
		}

		// child process
		else if (pid == 0) {
			if (redirect_in) {
				// open input file
				int fin = open(redirect_in_path.c_str(), O_RDONLY);
				dup2(fin, STDIN_FILENO);
				close(fin);
			}

			if (need_pipe) {
				// connect output of first command to fpipe[1]
				dup2(fpipe[1], STDOUT_FILENO);
				close(fpipe[0]);
				close(fpipe[1]);
			}

			else if (redirect_out) {
				// open or create output file in mode 644
				int fout = open(
					redirect_out_path.c_str(),
					O_WRONLY | O_CREAT,
					S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
				);
				dup2(fout, STDOUT_FILENO);
				close(fout);
			}

			if (blocking) {
				// the exec family: l for list, v for vector, p for path, e for environment
				execvp(c_arguments[0], c_arguments);
				// error occurred
				_exit(1);
			}

			else {
				pid_t pid_inner = fork();
				if (pid_inner < 0)
					_exit(1);
				else if (pid_inner == 0) {
					execvp(c_arguments[0], c_arguments);
					_exit(1);
				}
				else
					_exit(0);
			}
		}

		// parent process
		else {
			int status;
			waitpid(pid, &status, 0);
			// workaround for overlapped output
			usleep(100);
			if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0))
				std::cout << "shell: command execution failed: " << command << std::endl;

			for (int j = 0; j < arg_len; ++j)
				delete[] c_arguments[j];
			delete[] c_arguments;
		}
	}

}
