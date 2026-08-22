# applications/precision/elreal/CMakeLists.txt
#
# elreal demonstrations. Registered with "false": these are demonstrations, not
# regression tests -- they print and explain rather than assert, and converging a
# float host to a thousand digits takes tens of seconds, which does not belong in
# a test run. They are still BUILT, so they cannot silently rot.

file (GLOB SOURCES "./*.cpp")

compile_all("false" "elreal_demo" "Applications/Precision/elreal" "${SOURCES}")
