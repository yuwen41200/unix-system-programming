#include <sys/stat.h>
#include <fstream>

int main(int argc, char** argv) {
	std::ifstream in("data.txt", std::ios::in | std::ios::binary);
	std::ofstream out("new_data.txt", std::ios::out | std::ios::trunc | std::ios::binary);

	struct stat st;
	stat("data.txt", &st);
	int len = (int) st.st_size / 4096 * 4;
	out.write((char *) &len, sizeof(int));

	char buf[4096];
	in.read(buf, 4096);
	while (in.good()) {
		out.write(buf, 4);
		in.read(buf, 4096);
	}

	in.clear();
	in.seekg(0);

	in.read(buf, 4096);
	while (in.good()) {
		out.write(buf, 4096);
		in.read(buf, 4096);
	}

	out.close();
	in.close();
	return 0;
}
