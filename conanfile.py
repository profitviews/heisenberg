from conan import ConanFile
from conan.tools.cmake import CMakeDeps, CMakeToolchain
from conan.tools.env import VirtualBuildEnv


class HeisenbergConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"

    def requirements(self):
        self.requires("boost/1.80.0")
        self.requires("fmt/8.1.1")
        self.requires("openssl/1.1.1q")
        self.requires("rapidjson/cci.20211112")

    def build_requirements(self):
        self.tool_requires("cmake/3.27.0")
        self.tool_requires("ninja/1.11.1")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generator = "Ninja Multi-Config"
        tc.generate()
        VirtualBuildEnv(self).generate()
