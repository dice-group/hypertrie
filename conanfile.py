import os
import re

from conan import ConanFile, tools
from conan.tools.cmake import cmake_layout, CMake
from conan.tools.files import rmdir, load


class Recipe(ConanFile):
    author = "DICE Group <info@dice-research.org>"
    url = "https://tentris.dice-research.org"
    topics = "tensor", "data structure", "einsum", "einstein summation", "hypertrie", "query", "sparql"
    settings = "build_type", "compiler", "os", "arch"
    options = {
        "with_test_deps": [True, False],
    }
    default_options = {
        "with_test_deps": False,
    }

    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("robin-hood-hashing/3.11.5", transitive_headers=True)
        self.requires("dice-hash/0.4.6", transitive_headers=True)
        self.requires("dice-sparse-map/0.2.5", transitive_headers=True)
        self.requires("dice-template-library/1.9.1", transitive_headers=True)
        self.requires("boost/1.84.0", transitive_headers=True, libs=False, force=True)

        if self.options.with_test_deps:
            self.requires("fmt/8.0.1")
            self.requires("cppitertools/2.1")
            self.requires("doctest/2.4.11")
            self.requires("metall/0.26")

    def set_name(self):
        if not hasattr(self, 'name') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.name = re.search(r"project\(\s*([a-z\-]+)\s+VERSION", cmake_file).group(1)

    def set_version(self):
        if not hasattr(self, 'version') or self.version is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.version = re.search(r"project\([^)]*VERSION\s+(\d+\.\d+.\d+)[^)]*\)", cmake_file).group(1)
        if not hasattr(self, 'description') or self.description is None:
            cmake_file = load(self, os.path.join(self.recipe_folder, "CMakeLists.txt"))
            self.description = re.search(r"project\([^)]*DESCRIPTION\s+\"([^\"]+)\"[^)]*\)", cmake_file).group(1)

    def layout(self):
        cmake_layout(self)

    _cmake = None

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.configure()
        return self._cmake

    def build(self):
        self._configure_cmake().build()

    def package(self):
        self._configure_cmake().install()
        for dir in ("res", "share", "cmake"):
            tools.files.rmdir(self, os.path.join(self.package_folder, dir))
        tools.files.copy(self, "LICENSE", src=self.folders.base_source, dst="licenses")

    def package_info(self):
        main_component = self.name
        self.cpp_info.set_property("cmake_target_name", f"{self.name}")
        self.cpp_info.components["global"].set_property("cmake_target_name", f"{self.name}::{main_component}")
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = f"{self.name}"
        self.cpp_info.components["global"].names["cmake_find_package"] = f"{self.name}"
        self.cpp_info.set_property("cmake_file_name", f"{self.name}")
        self.cpp_info.components["global"].includedirs = [f"include/{self.name}/{main_component}/"]
        self.cpp_info.components["global"].libdirs = []
        self.cpp_info.components["global"].bindirs = []
        self.cpp_info.components["global"].requires = [
            "dice-hash::dice-hash",
            "dice-sparse-map::dice-sparse-map",
            "dice-template-library::dice-template-library",
            "robin-hood-hashing::robin-hood-hashing",
            "boost::headers",
        ]
        if self.options.with_test_deps:
            self.cpp_info.components["global"].requires.append("fmt::fmt")
            self.cpp_info.components["global"].requires.append("metall::metall")
            self.cpp_info.components["global"].requires.append("cppitertools::cppitertools")
            self.cpp_info.components["global"].requires.append("doctest::doctest")

        for component in ("einsum", "query"):
            self.cpp_info.components[f"{component}"].includedirs = [f"include/{self.name}/{component}"]
            self.cpp_info.components[f"{component}"].names["cmake_find_package_multi"] = f"{component}"
            self.cpp_info.components[f"{component}"].names["cmake_find_package"] = f"{component}"
