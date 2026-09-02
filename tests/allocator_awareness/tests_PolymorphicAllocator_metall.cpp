#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <fmt/format.h>
#include <random>

#include <cassert>
#include <unistd.h>
#include <sys/wait.h>

#ifdef NDEBUG
#warning "This test does almost nothing if it is compiled with NDEBUG"
#endif

TEST_CASE("PolymorphicAllocator metall") {
	std::string const path{fmt::format("/tmp/{}", std::random_device{}())};

	auto pid = fork();
	assert(pid >= 0);

	if (pid == 0) {
		int st = execl("PolymorphicAllocator_metall_phase1", "PolymorphicAllocator_metall_phase1", path.data(), nullptr);
		assert(st == 0);
	} else {
		int rc;
		int st = waitpid(pid, &rc, 0);
		assert(st >= 0);
		assert(WIFEXITED(rc));
		assert(WEXITSTATUS(rc) == 0);
	}

	pid = fork();
	assert(pid >= 0);

	if (pid == 0) {
		int st = execl("PolymorphicAllocator_metall_phase2", "PolymorphicAllocator_metall_phase2", path.data(), nullptr);
		assert(st == 0);
	} else {
		int rc;
		int st = waitpid(pid, &rc, 0);
		assert(st >= 0);
		assert(WIFEXITED(rc));
		assert(WEXITSTATUS(rc) == 0);
	}
}
