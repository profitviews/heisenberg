from conan import ConanFile
from conan.errors import ConanException, ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMake, CMakeDeps, CMakeToolchain
from conan.tools.scm import Version
import re, os.path
import subprocess
import sys

def get_cmake_version():
    try:
        out, _ = subprocess.Popen(["cmake", "--version"], stdout=subprocess.PIPE, shell=False).communicate()
        out = out.decode(sys.stdout.encoding)
        version_line = out.split('\n', 1)[0]
        version_str = version_line.rsplit(' ', 1)[-1]
        return Version(version_str)
    except Exception as e:
        raise ConanException("Error retrieving CMake version: '{}'".format(e))

class CppCryptoAlgos(ConanFile):
    name = "cpp_crypto_algos"
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    exports_sources = ["CMakeLists.txt", "cmake/*", "src/*"]
    requires = (
        "boost/1.78.0",
        "fmt/[8.1.1]",
        "openssl/1.1.1q",
        "rapidjson/cci.20211112"
    )

    def build_requirements(self):
        self.tool_requires("ninja/1.11.1")
        if get_cmake_version() < Version("3.27.0"):
            self.tool_requires("cmake/3.27.0")

    @property
    def _minimum_cpp_standard(self):
        return 20

    @property
    def _minimum_compilers_version(self):
        return {
            "msvc": "16",
            "gcc": "11",
            "clang": "13"
        }

    def configure(self):
        if self.settings.compiler.get_safe("cppstd"):
            check_min_cppstd(self, self._minimum_cpp_standard)
        min_version = self._minimum_compilers_version.get(
            str(self.settings.compiler))
        if not min_version:
            self.output.warn("{} recipe lacks information about the {} "
                             "compiler support.".format(
                                 self.name, self.settings.compiler))
        else:
            if Version(self.settings.compiler.version) < min_version:
                raise ConanInvalidConfiguration(
                    "{} requires C++{} support. "
                    "The current compiler {} {} does not support it.".format(
                        self.name, self._minimum_cpp_standard,
                        self.settings.compiler,
                        self.settings.compiler.version))

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()
        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
        cmake_layout(self)

    def package(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.install()
