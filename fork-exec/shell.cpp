#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

int main() {

	while (1) {
		std::cout << "> ";
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

		char **c_arguments = new char*[arguments.size() + 1];
		int i = 0;
		for (std::vector<std::string>::iterator it = arguments.begin(); it != arguments.end(); ++it) {
			char *c_argument = new char[it->size() + 1];
			std::copy(it->begin(), it->end(), c_argument);
			c_argument[it->size()] = '\0';
			c_arguments[i++] = c_argument;
		}
		c_arguments[i++] = nullptr;

		if (blocking)
			signal(SIGCHLD, SIG_DFL);
		else
			signal(SIGCHLD, SIG_IGN);

		pid_t pid = fork();
		if (pid < 0)
			std::cerr << "fork() error" << std::endl;

		else if (pid == 0) {
			// child process
			execvp(c_arguments[0], c_arguments);
			// the exec family: l for list, v for vector, p for path, e for environment
			_exit(1);
			// error occurred
		}

		else {
			if (blocking) {
				int status;
				waitpid(pid, &status, 0);
				if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0))
					std::cout << "shell: command execution failed: " << command << std::endl;
			}

			for (int j = 0; j < i; ++j)
				delete[] c_arguments[j];
			delete[] c_arguments;
		}
	}

}
