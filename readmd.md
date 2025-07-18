conan install . -s build_type=Debug --profile:host profiles/macos --profile:build profiles/macos --build=missing
conan build . -s build_type=Debug --profile:host profiles/macos --profile:build profiles/macos --build=missing
conan build . -s build_type=Release --profile:host profiles/macos --profile:build profiles/macos --build=missing