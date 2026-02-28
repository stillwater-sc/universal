# Linux Package engineering

Creating Linux packages for your Universal Number Library involves several interconnected processes. Lets break down the comprehensive workflow for packaging, building, testing, and distributing your header-only C++ library across major Linux distributions.

## Package Format Selection

Since we're targeting multiple distributions, we'll need to support different package formats:

- **DEB packages** (Debian, Ubuntu, derivatives)
- **RPM packages** (Red Hat, Fedora, SUSE, derivatives) 
- **AUR packages** (Arch Linux)
- **Snap/Flatpak** (universal, though less common for development libraries)

For a header-only library with CLI utilities, DEB and RPM will be your primary targets.

## Build System Integration

Given our modern C++ focus, integrate packaging into the existing build system:

### CMake Integration

```cmake
# Use CPack for cross-platform packaging
include(CPack)
set(CPACK_PACKAGE_NAME "universal-number-lib")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_GENERATOR "DEB;RPM")

# Separate packages per number system
set(CPACK_COMPONENTS_ALL headers cli-utils)
set(CPACK_DEB_COMPONENT_INSTALL ON)
set(CPACK_RPM_COMPONENT_INSTALL ON)
```

## Package Architecture Strategy

For modularity, consider this structure:

- `universal-number-lib-dev` - Core headers and CMake configs
- `universal-number-lib-posit` - Posit arithmetic headers
- `universal-number-lib-fixpnt` - Fixed-point headers
- `universal-number-lib-tools` - CLI utilities
- Individual packages per number system as needed

## Build Process

### Automated CI/CD Pipeline

1. **GitHub Actions/GitLab CI** for automated builds
2. **Multi-architecture builds** (x86_64, ARM64, potentially RISC-V)
3. **Cross-compilation** using Docker containers with different base images
4. **Package validation** using lintian (DEB) and rpmlint (RPM)

### Sample workflow structure

```yaml
- Build and test on Ubuntu LTS, Fedora, Arch
- Generate packages for each architecture
- Run package installation tests
- Upload to staging repositories
```

## Quality Assurance Process

### Pre-release QA

- **Static analysis** integration (clang-tidy, PVS-Studio)
- **Package linting** (lintian, rpmlint, namcap)
- **Installation testing** on clean VMs/containers
- **Reverse dependency testing** if other packages depend on yours
- **ABI compatibility checking** using abidiff tools

### Testing matrix

- Multiple compiler versions (GCC 9-13, Clang 10-17)
- Different C++ standard versions (C++17, C++20, C++23)
- Various optimization levels and sanitizer builds

## Distribution Registration Process

### Debian/Ubuntu

1. **Mentors process** - Upload to mentors.debian.net for sponsor review
2. **ITP bug filing** (Intent To Package) in Debian BTS
3. **Packaging standards compliance** (Debian Policy, debhelper compat levels)
4. **Sponsor finding** for initial uploads until you become a Debian Developer

### Fedora

1. **Package Review Process** - Submit spec file for community review
2. **Fedora Package Database** registration
3. **Bugzilla integration** for package maintenance
4. **Koji build system** integration


### Arch Linux

1. **AUR submission** - Create PKGBUILD and submit to AUR
2. **Community maintenance** or eventual migration to official repositories

## Release Management



**Versioning Strategy:**

- Semantic versioning aligned with your library releases

- Package revision numbers for packaging-only changes

- Epoch handling for version scheme changes



**Release Automation:**

```bash
# Automated release pipeline

1. Tag release in Git

2. Trigger CI builds for all targets

3. Generate changelogs from Git history

4. Upload packages to staging

5. Run integration tests

6. Promote to production repositories

```

## Specialized Considerations for Header-Only Libraries

### Package Content

- Install headers to `/usr/include/universal/`
- CMake config files in `/usr/lib/cmake/universal/`
- pkg-config files for non-CMake builds
- Documentation and examples

### Development Experience

```cmake
# Users should be able to simply:
find_package(Universal COMPONENTS posit fixpnt REQUIRED)

target_link_libraries(myapp Universal::posit Universal::fixpnt)

```
## Repository Hosting Options



### Personal Package Archives (PPAs)

- Launchpad PPAs for Ubuntu
- Copr for Fedora/RHEL
- OBS (Open Build Service) for cross-distribution builds

### Corporate/Professional

- JFrog Artifactory
- Sonatype Nexus
- CloudSmith for cloud-native package management

## Long-term Maintenance Strategy

### Automated maintenance

- Dependabot/Renovate for dependency updates
- Automated security scanning
- License compliance checking
- Documentation generation and deployment

### Community engagement

- Clear contribution guidelines
- Issue templates for packaging problems
- Distribution-specific maintainer relationships

The complexity scales with the ambition to provide a easy to use mixed-precision algorithm development and optimization SDK. Starting with PPA/Copr for testing, then moving to official repositories as adoption grows. Given the specialized nature of numerical computing libraries, consider also targeting HPC-focused distributions and package managers like Spack, which are popular in scientific computing communities where your library would see heavy use.


