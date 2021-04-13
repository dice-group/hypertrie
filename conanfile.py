import os
import re
from conans import ConanFile, CMake
from conans.tools import load


class Hypertrie(ConanFile):
    name = "hypertrie"
    author = "DICE Group <info@dice-research.org>"
    description = "A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (einsum)"
    homepage = "https://github.com/dice-group/hypertrie"
    url = homepage
    license = "AGPL"
    topics = "tensor", "data structure", "einsum", "einstein summation", "hypertrie"
    settings = "build_type", "compiler", "os", "arch"
    requires = (
        "range-v3/0.11.0",
        "boost/1.75.0",
        "fmt/7.1.3",
        "tsl-hopscotch-map/2.3.0",
        "tsl-sparse-map/0.6.2",
        "robin-hood-hashing/3.9.1",
        "dice-hash/0.1.0@dice-group/stable"
    )
    generators = "cmake", "cmake_find_package", "cmake_paths"
    exports = "LICENSE.txt"
    exports_sources = "include/*", "CMakeLists.txt", "cmake/*"
    no_copy_source = True

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search("project\(hypertrie VERSION (.*)\)", cmake_file).group(1)

    def package(self):
        cmake = CMake(self)
        cmake.definitions["hypertrie_BUILD_TESTS"] = "OFF"
        cmake.configure()
        cmake.install()

    def package_id(self):
        self.info.header_only()

    def imports(self):
        self.copy("license*", dst="licenses", folder=True, ignore_case=True)
