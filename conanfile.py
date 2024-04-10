import os
import re

from conans import ConanFile, load, CMake
from conans.util.files import rmdir


class Hypertrie(ConanFile):
    author = "DICE Group <info@dice-research.org>"
    homepage = "https://github.com/dice-group/hypertrie"
    url = homepage
    topics = "tensor", "data structure", "einsum", "einstein summation", "hypertrie", "query", "sparql"
    settings = "build_type", "compiler", "os", "arch"
    generators = "cmake_find_package"
    options = {"with_test_deps": [True, False]}
    default_options = {"with_test_deps": False}

    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"

    def requirements(self):
        reqs = (
            "boost/1.84.0",
            "robin-hood-hashing/3.11.5",
            "dice-hash/0.4.0",
            "dice-sparse-map/0.2.1",
            "dice-template-library/0.2.0"
        )
        for req in reqs:
            self.requires(req)
        if self.options.with_test_deps:
            self.requires("fmt/8.0.1")
            self.requires("metall/0.20")
            self.requires("cppitertools/2.1")
            self.requires("doctest/2.4.6")

    def set_name(self):
        if not hasattr(self, 'name') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.name = re.search(r"project\(\s*([a-z\-]+)\s+VERSION", cmake_file).group(1)

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)
        if not hasattr(self, 'description') or self.description is None:
            cmake_file = load(os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.description = re.search(r"project\([^)]*DESCRIPTION\s+\"([^\"]+)\"[^)]*\)", cmake_file).group(1)

    def build(self):
        cmake = CMake(self)
        cmake.definitions['CONAN_CMAKE'] = False
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        for dir in ("lib", "res", "share"):
            rmdir(os.path.join(self.package_folder, dir))

    def package_info(self):  #
        # self.cpp_info.set_property("cmake_target_name", "hypertrie")
        self.cpp_info.components["global"].set_property("cmake_target_name", "hypertrie::hypertrie")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = "hypertrie"
        self.cpp_info.components["global"].names["cmake_find_package"] = "hypertrie"
        self.cpp_info.set_property("cmake_file_name", "hypertrie")
        self.cpp_info.components["global"].includedirs = ["include/hypertrie/hypertrie/"]
        self.cpp_info.components["global"].requires = [
            "dice-hash::dice-hash",
            "dice-sparse-map::dice-sparse-map",
            "boost::boost",
            "robin-hood-hashing::robin-hood-hashing",
            "dice-template-library::dice-template-library"
        ]
        if self.options.with_test_deps:
            self.cpp_info.components["global"].requires.append("fmt::fmt")
            self.cpp_info.components["global"].requires.append("metall::metall")
            self.cpp_info.components["global"].requires.append("cppitertools::cppitertools")
            self.cpp_info.components["global"].requires.append("doctest::doctest")

        for component in ["einsum", "query"]:
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"
            self.cpp_info.components[f"{component}"].includedirs = [f"include/hypertrie/{component}"]
            self.cpp_info.components[f"{component}"].requires = (
                "global",
            )
