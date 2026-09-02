import os
import re

from conan import ConanFile
from conan.tools.cmake import CMake
from conan.tools.files import rmdir, load


class Hypertrie(ConanFile):
    author = "DICE Group <info@dice-research.org>"
    homepage = "https://github.com/dice-group/hypertrie"
    url = homepage
    topics = "tensor", "data structure", "einsum", "einstein summation", "hypertrie"
    settings = "build_type", "compiler", "os", "arch"
    generators = ("CMakeDeps", "CMakeToolchain")
    options = {"with_test_deps": [True, False], "build_ffi": [True, False]}
    default_options = {
        "with_test_deps": False,
        "build_ffi": False,
    }

    exports_sources = "libs/*", "CMakeLists.txt", "cmake/*"

    def requirements(self):
        self.requires("boost/1.81.0")
        self.requires("dice-hash/0.4.3")
        self.requires("dice-sparse-map/0.2.4")
        self.requires("dice-template-library/1.1.0")
        self.requires("unordered_dense/4.0.4")

        if self.options.with_test_deps:
            self.requires("fmt/8.0.1")
            self.requires("cppitertools/2.1")
            self.requires("doctest/2.4.11")
            self.requires("metall/0.21")

        if self.options.build_ffi:
            self.requires("metall-ffi/0.2.0")

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

    _cmake = None

    def _configure_cmake(self):
        if self._cmake is None:
            self._cmake = CMake(self)
            self._cmake.configure(variables={"USE_CONAN": False, "BUILD_FFI": self.options.build_ffi})

        return self._cmake

    def build(self):
        self._configure_cmake().build()

    def package(self):
        self._configure_cmake().install()
        for dir in ("res", "share"):
            rmdir(self, os.path.join(self.package_folder, dir))

    def package_info(self):
        self.cpp_info.set_property("cmake_file_name", f"{self.name}")

        self.cpp_info.components["global"].set_property("cmake_target_name", f"{self.name}::{self.name}")
        self.cpp_info.components["global"].includedirs = [f"include/{self.name}/{self.name}/"]
        self.cpp_info.components["global"].names["cmake_find_package_multi"] = f"{self.name}"
        self.cpp_info.components["global"].names["cmake_find_package"] = f"{self.name}"
        self.cpp_info.components["global"].requires = [
            "dice-hash::dice-hash",
            "dice-sparse-map::dice-sparse-map",
            "dice-template-library::dice-template-library",
            "boost::headers",
            "unordered_dense::unordered_dense",
        ]

        if self.options.with_test_deps:
            self.cpp_info.components["global"].requires += [
                "fmt::fmt",
                "cppitertools::cppitertools",
                "doctest::doctest",
                "metall::metall",
            ]

        self.cpp_info.components["einsum"].requires = [
            "global",
        ]

        self.cpp_info.components["einsum"].set_property("cmake_target_name", f"{self.name}::einsum")
        self.cpp_info.components["einsum"].includedirs = [f"include/{self.name}/einsum"]
        self.cpp_info.components["einsum"].names["cmake_find_package_multi"] = "einsum"
        self.cpp_info.components["einsum"].names["cmake_find_package"] = "einsum"

        if self.options.build_ffi:
            self.cpp_info.components["ffi"].requires = (
                "global",
                "einsum",
                "metall-ffi::metall-ffi",
            )

            original_cmake_name = f"{self.name}-ffi"
            self.cpp_info.components["ffi"].set_property("cmake_target_name", f"{self.name}::ffi")
            self.cpp_info.components["ffi"].includedirs = [f"include/{self.name}/{original_cmake_name}"]
            self.cpp_info.components["ffi"].libdirs = [f"lib/{self.name}/{original_cmake_name}"]
            self.cpp_info.components["ffi"].libs = [f"{original_cmake_name}"]
            self.cpp_info.components["ffi"].names["cmake_find_package_multi"] = "ffi"
            self.cpp_info.components["ffi"].names["cmake_find_package"] = "ffi"
