#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <Dice/hypertrie/hypertrie.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <absl/hash/hash.h>
#include <fmt/format.h>


using PhysicalMem = uint32_t;


inline int parseLine(char *line) {
	// This assumes that a digit will be found and the line ends in " Kb".
	int i = strlen(line);
	const char *p = line;
	while (*p < '0' || *p > '9') p++;
	line[i - 3] = '\0';
	i = atoi(p);
	return i;
}

inline PhysicalMem get_memory_usage() {
	std::FILE *file = std::fopen("/proc/self/status", "r");
	char line[128];
	PhysicalMem physicalMem{};

	while (std::fgets(line, 128, file) != NULL) {

		if (std::strncmp(line, "VmRSS:", 6) == 0) {
			physicalMem = parseLine(line);
		}
	}
	std::fclose(file);
	return physicalMem;
}


int main(int argc, char *argv[]) {

	using namespace hypertrie;
	using namespace fmt::literals;
	using namespace std::chrono;
	if (argc != 2) {
		std::cerr << "Please provide exactly one CSV file with triple IDS only and no headings." << std::endl;
		exit(EXIT_FAILURE);
	}

	std::string rdf_file{argv[1]};
	if (not std::filesystem::is_regular_file(rdf_file)) {
		std::cerr << "{} is not a file."_format(rdf_file) << std::endl;
		exit(EXIT_FAILURE);
	}

	using tr = Hypertrie_t<unsigned long,
			bool,
			hypertrie::internal::container::tsl_sparse_map,
			hypertrie::internal::container::tsl_sparse_set,
			false>;
	constexpr hypertrie::pos_type depth = 3;

	using key_part_type = typename tr::key_part_type;
	using Key = typename tr::Key;

	// create emtpy primary node
	Hypertrie<tr> hypertrie (depth);

	BulkInserter<tr> bulk_inserter{hypertrie};

	std::ifstream file(rdf_file);

	std::string line = "";
	// Iterate through each line and split the content using delimeter
	unsigned int total = 0;
	auto start = steady_clock::now();
	{
		BulkInserter<tr> bulk_inserter{hypertrie};
		while (getline(file, line)) {
			++total;
			using boost::lexical_cast;
			std::vector<std::string> id_triple;
			// std::cout << line << std::endl;
			boost::algorithm::split(id_triple, line, boost::algorithm::is_any_of(","));
			//		std::cout << count << " #    # " << line << std::endl;

			Key key{lexical_cast<key_part_type>(id_triple[0]), lexical_cast<key_part_type>(id_triple[1]), lexical_cast<key_part_type>(id_triple[2])};
			bulk_inserter.add(std::move(key));
		}
	}
	auto end = steady_clock::now();
	file.close();
	std::cerr << "{:} mio triples processed."_format(double(total)) << std::endl;
	std::cerr << "hypertrie entries: {:d}."_format(hypertrie.size()) << std::endl;
	std::cerr << "hypertrie size estimation: {:d} kB."_format(get_memory_usage()) << std::endl;
	auto duration = end - start;

	std::cerr << "duration: {:d}.{:03d} s."_format(duration_cast<seconds>(duration).count(),
												   (duration_cast<milliseconds>(duration) % 1000).count())
			  << std::endl;
}
