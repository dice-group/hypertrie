#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <cstdio>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <Dice/hypertrie/internal/node_based/NodeContext.hpp>

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

	using namespace hypertrie::internal::node_based;
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

	using tr = default_bool_Hypertrie_internal_t;
	constexpr hypertrie::pos_type depth = 3;

	using key_part_type = typename tr::key_part_type;
	using value_type = typename tr::value_type;
	using Key = typename tr::template RawKey<depth>;

	NodeContext<depth, tr> context{};
	// create emtpy primary node
	UncompressedNodeContainer<depth, tr> hypertrie = context.template newPrimaryNode<depth>();

	std::ifstream file(rdf_file);

	std::string line = "";
	// Iterate through each line and split the content using delimeter
	unsigned int total = 0;
	unsigned int count = 0;
	unsigned int _1mios = 0;
	auto start = steady_clock::now();
	while (getline(file, line)) {
		++count;
		++total;
		using boost::lexical_cast;
		std::vector<std::string> id_triple;
		// std::cout << line << std::endl;
		boost::algorithm::split(id_triple, line, boost::algorithm::is_any_of(","));

		context.template set<depth>(hypertrie, Key{lexical_cast<key_part_type>(id_triple[0]),
										   lexical_cast<key_part_type>(id_triple[1]),
										   lexical_cast<key_part_type>(id_triple[2])}, true);

		if (count == 1'000'000) {
			count = 0;
			++_1mios;
			std::cerr << "{:d} mio triples processed."_format(_1mios/1'000'000) << std::endl;
		}
	}
	auto end = steady_clock::now();
	file.close();
	std::cerr << "{:} mio triples processed."_format(double(total)/1'000'000) << std::endl;
	std::cerr << "hypertrie entries: {:d}."_format(hypertrie.node()->size()) << std::endl;
	std::cerr << "hypertrie size estimation: {:d} kB."_format(get_memory_usage()) << std::endl;
	auto duration = end - start;

	std::cerr << "duration: {:d}.{:04d} s."_format(duration_cast<seconds>(duration).count(),
												   (duration_cast<milliseconds>(duration) % 1000).count()) << std::endl;
}
