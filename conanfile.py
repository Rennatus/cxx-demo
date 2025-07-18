from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
import os

class myappRecipe(ConanFile):
    name = "cxx-demo"
    version = "0.1"
    #package_type = "application"
    # Optional metadata
    license = "<Put the package license here>"
    author = "<Put your name here> <And your email here>"
    url = "<Package recipe repository url here, for issues about the package>"
    description = "<Description of myapp package here>"
    topics = ("<Put some tag here>", "<here>", "<and here>")
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
        "with_tests": [True, False],
        "with_benchmarks": [True, False]
    }
    default_options = {
        "shared": False,
        "fPIC": True,
        "with_tests": True,
        "with_benchmarks": False,
        "spdlog/*:header_only": True,
        "catch2/*:header_only": True,
        "catch2/*:with_main": True
    }
    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "src/*", "test/*", "benchmark/*", "CMakeLists.txt"
    def layout(self):
        cmake_layout(self)
    def requirements(self):
        self.requires("spdlog/1.15.3")      
        if self.options.with_tests:
            self.requires("catch2/3.8.1")
        if self.options.with_benchmarks:
            self.requires("benchmark/1.9.4")
    def configure(self):
        # 如果是静态库且在Linux上，启用fPIC
        if self.options.shared == False:
            self.options.fPIC = True

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        build_type = self.settings.build_type
        if (build_type == "Debug"):
            tc.variables["CMAKE_EXPORT_COMPILE_COMMANDS"] = True
        if (self.options.with_tests):
            tc.variables["BUILD_TESTS"] = True
        tc.generate()
       

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        
        source_path = os.path.join(self.build_folder, "compile_commands.json")
        target_path = os.path.join(self.source_folder, "build","compile_commands.json")
        if os.path.exists(source_path):
            copy(self, "compile_commands.json", 
             src=os.path.dirname(source_path), 
             dst=os.path.dirname(target_path))

    def package(self):
        cmake = CMake(self)
        cmake.install()

