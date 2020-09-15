from conans import ConanFile, CMake
from conans.tools import load
import re, os


class Hypertrie(ConanFile):
    name = "hypertrie"
    author = "DICE Group <info@dice-research.org>"
    description = "A flexible data structure for low-rank, sparse tensors supporting slices by any dimension and einstein summation (einsum)"
    homepage = "https://github.com/dice-group/hypertrie"
    url = homepage
    license = "AGPL"
    topics = "tensor", "data structure", "einsum", "einstein summation", "hypertrie"
    settings = "build_type", "compiler", "os", "arch"
    requires = "boost/1.74.0", "tsl-hopscotch-map/2.2.1@tessil/stable", "fmt/7.0.3", "abseil/20200225.2", "Catch2/2.11.1@catchorg/stable", "tsl-sparse-map/0.6.2@tessil/stable", "robin-hood-hashing/3.7.0"
    generators = "cmake", "cmake_find_package", "cmake_paths"
    exports = "LICENSE.txt"
    exports_sources = "include/*", "thirdparty/*", "CMakeLists.txt", "cmake/*"
    no_copy_source = True

    def set_version(self):
        env_version = os.getenv("hypertrie_deploy_version", False)
        if env_version:
            self.version = env_version
        else:
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
