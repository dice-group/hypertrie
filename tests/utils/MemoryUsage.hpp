#ifndef HYPERTRIE_MEMORYUSAGE_HPP
#define HYPERTRIE_MEMORYUSAGE_HPP


namespace hypertrie::tests::utils {
	struct processMem_t {
		uint32_t virtualMem;
		uint32_t physicalMem;
	};


	inline int parseLine(char *line) {
		// This assumes that a digit will be found and the line ends in " Kb".
		int i = strlen(line);
		const char *p = line;
		while (*p < '0' || *p > '9') p++;
		line[i - 3] = '\0';
		i = atoi(p);
		return i;
	}

	inline processMem_t get_memory_usage() {
		FILE *file = fopen("/proc/self/status", "r");
		char line[128];
		processMem_t processMem{};

		while (fgets(line, 128, file) != nullptr) {
			// std::cout << line << std::endl;
			if (strncmp(line, "VmSize:", 7) == 0) {
				processMem.virtualMem = parseLine(line);
			}

			if (strncmp(line, "VmRSS:", 6) == 0) {
				processMem.physicalMem = parseLine(line);
			}
		}
		fclose(file);
		return processMem;
	}
}

#endif //HYPERTRIE_MEMORYUSAGE_HPP
