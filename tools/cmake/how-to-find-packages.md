# How find_package() works

## Ranked list of directories

How find_package(universal CONFIG) found an old install at C:/Program Files (x86)/universal

CMake's CONFIG-mode lookup on Windows searches a ranked list of places for a file named <Name>Config.cmake or <name>-config.cmake. In priority order, the
ones that matter here are:

1. universal_ROOT / universal_DIR (CMake vars or env vars) — explicit pin
2. CMAKE_PREFIX_PATH (CMake var or env var)
3. PATH (CMake walks up to parents of each entry)
4. Windows user package registry: HKCU\Software\Kitware\CMake\Packages\universal
5. CMAKE_SYSTEM_PREFIX_PATH — which on Windows includes C:\Program Files\ and C:\Program Files (x86)\ by default
6. Windows system package registry: HKLM\Software\Kitware\CMake\Packages\universal

Within each prefix, CMake tries suffix patterns including:

- <prefix>/<name>*/
- <prefix>/<name>*/cmake/ or CMake/
- <prefix>/<name>*/(lib|lib/<arch>|share)/cmake/<name>*/

Universal's CMakeLists.txt:701-704 installs the config to <prefix>/CMake/ on Windows. So with a default cmake --install the layout is:

C:\Program Files (x86)\universal\
  Include\sw\...
  CMake\universal-config.cmake
  CMake\universal-config-version.cmake
  CMake\universal-targets.cmake
  lib\posit_c_api_pure.lib

That matches the <prefix>/<name>*/CMake/ pattern against step 5 (C:\Program Files (x86)\), so find_package(universal CONFIG) finds it with no hints at all.
No registry entry needed — Universal doesn't call export(PACKAGE …), so the HKCU registry path isn't used here.

## How to remove the stale install cleanly

Since you don't have the old build tree with install_manifest.txt, make uninstall / cmake --build . --target uninstall can't work. You need to delete by
hand, and the things to hit are:

1. The install directory itself (this is the main one):
```bash
rmdir /S /Q "C:\Program Files (x86)\universal"
```

That wipes headers, the CMake\ config triplet, and the C-API static libs. Requires an elevated shell.

2. Check the CMake package registry (belt-and-suspenders — export(PACKAGE) isn't used today, but an older Universal might have called it):
```ps
reg query "HKCU\Software\Kitware\CMake\Packages\universal" 2>nul
reg query "HKCU\Software\Kitware\CMake\Packages\UNIVERSAL" 2>nul
reg query "HKLM\Software\Kitware\CMake\Packages\universal" 2>nul
```
If any of those return entries, delete the key — each value under that key is a path to a build tree that was registered via export(PACKAGE). 
Stale entries can silently redirect future find_package calls to phantom build dirs.

3. Environment variables:
- CMAKE_PREFIX_PATH (user + system)
- universal_DIR, UNIVERSAL_DIR, universal_ROOT
Any that mention the old install should be cleared.

4. Consumer CMake caches:
Anything that previously did find_package(universal) (i.e. mp-dsp-python) has cached the old path in CMakeCache.txt. After removing the install, either:
- cmake --fresh -S . -B build (CMake 3.24+), or
- delete the build directory and reconfigure.

Otherwise the cache will stubbornly point at the now-nonexistent directory until something errors.

Sanity check after cleanup — run this from a fresh shell:
cmake --find-package -DNAME=universal -DCOMPILER_ID=MSVC -DLANGUAGE=CXX -DMODE=EXIST
Should print universal not found. If it still finds something, one of the four sources above is still dirty.

Preventing drift going forward

For mp-dsp-python I'd recommend pinning explicitly rather than relying on system discovery — set universal_DIR to the specific install you want, or use
FetchContent / add_subdirectory against the Universal repo directly. That way a stale C:\Program Files (x86) leftover can never shadow the intended
version.

