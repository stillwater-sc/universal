file (GLOB SOURCES "./*.cpp")

# Use "false" for testing - these are demos that require user-provided input images
compile_all("false" "app_img" "Applications/Mixed Precision/Imaging" "${SOURCES}")
