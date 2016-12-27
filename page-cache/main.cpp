#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <list>
#include <map>

typedef unsigned long page_no_t;
typedef unsigned long page_count_t;

class Cache {
protected:
	std::map<page_no_t, std::list<page_no_t>::iterator> page_map;
	std::list<page_no_t> page_list;
	page_count_t total_frame;
public:
	Cache(page_count_t total_frame): total_frame(total_frame) {}
	virtual bool lookup(page_no_t page_no) = 0;
	virtual void replace(page_no_t page_no) = 0;
	virtual ~Cache() {}
};

class Fifo: public Cache {
public:
	Fifo(page_count_t total_frame): Cache(total_frame) {}
	bool lookup(page_no_t page_no);
	void replace(page_no_t page_no);
};

class Lru: public Cache {
public:
	Lru(page_count_t total_frame): Cache(total_frame) {}
	bool lookup(page_no_t page_no);
	void replace(page_no_t page_no);
};

int main() {
	Cache *cache;
	std::cout << std::fixed << std::setprecision(9);
	std::ifstream trace_file("trace.txt");

	if (!trace_file.is_open()) {
		std::cerr << "cannot open trace file" << std::endl;
		return 1;
	}

	std::cout << "FIFO---" << std::endl << "size\tmiss\thit\tpage fault ratio" << std::endl;
	for (page_count_t total_frame = 64; total_frame <= 512; total_frame *= 2) {
		cache = new Fifo(total_frame);
		page_count_t hit_count = 0, miss_count = 0;

		for (std::string record; std::getline(trace_file, record); ) {
			page_no_t page_no = std::stoul(record.substr(3, 5), nullptr, 16);
			if (cache->lookup(page_no))
				hit_count++;
			else {
				cache->replace(page_no);
				miss_count++;
			}
		}

		std::cout << total_frame << "\t" << miss_count << "\t" << hit_count << "\t";
		std::cout << (long double) miss_count / (hit_count + miss_count) << std::endl;

		trace_file.clear();
		trace_file.seekg(0);
		delete cache;
	}

	std::cout << "LRU---" << std::endl << "size\tmiss\thit\tpage fault ratio" << std::endl;
	for (page_count_t total_frame = 64; total_frame <= 512; total_frame *= 2) {
		cache = new Lru(total_frame);
		page_count_t hit_count = 0, miss_count = 0;

		for (std::string record; std::getline(trace_file, record); ) {
			page_no_t page_no = std::stoul(record.substr(3, 5), nullptr, 16);
			if (cache->lookup(page_no))
				hit_count++;
			else {
				cache->replace(page_no);
				miss_count++;
			}
		}

		std::cout << total_frame << "\t" << miss_count << "\t" << hit_count << "\t";
		std::cout << (long double) miss_count / (hit_count + miss_count) << std::endl;

		trace_file.clear();
		trace_file.seekg(0);
		delete cache;
	}

	trace_file.close();
	return 0;
}

bool Fifo::lookup(page_no_t page_no) {
	return page_map.find(page_no) != page_map.end();
}

void Fifo::replace(page_no_t page_no) {
	if (page_list.size() < total_frame) {
		page_list.push_back(page_no);
		auto iterator = page_list.end();
		page_map.insert(std::make_pair(page_no, --iterator));
	}
	else {
		page_map.erase(page_list.front());
		page_list.pop_front();
		replace(page_no);
	}
}

bool Lru::lookup(page_no_t page_no) {
	auto renewed_page = page_map.find(page_no);
	if (renewed_page != page_map.end()) {
		page_list.erase(renewed_page->second);
		page_list.push_back(renewed_page->first);
		auto iterator = page_list.end();
		renewed_page->second = --iterator;
		return true;
	}
	else
		return false;
}

void Lru::replace(page_no_t page_no) {
	if (page_list.size() < total_frame) {
		page_list.push_back(page_no);
		auto iterator = page_list.end();
		page_map.insert(std::make_pair(page_no, --iterator));
	}
	else {
		page_map.erase(page_list.front());
		page_list.pop_front();
		replace(page_no);
	}
}
