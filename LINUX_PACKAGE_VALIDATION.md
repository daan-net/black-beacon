# Linux Package Validation Report

## Build Information
- **Source Commit**: `b232ff1`
- **Branch**: `work/linux-tester-package`
- **Unreal Packaging Command**: 
  `/home/a1/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -project="/home/a1/WORK/BLACK_BEACON/BlackBeacon.uproject" -noP4 -platform=Linux -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory="/home/a1/WORK/BLACK_BEACON/dist/temp"`

## Package Details
- **Archive Filename**: `BlackBeacon-linux-x86_64-test1.tar.zst`
- **Compressed Size**: 480 MB
- **Unpacked Size**: 1.2 GB
- **SHA-256 Checksum**: `aa38db8044d70f587d964aa33dbbe540fd20427cf9a3410918c552371fc535ae`

## Tests Executed (Clean User Home Environment)
1. Fresh install ✔️
2. Launcher command exists ✔️
3. Game executable starts (verified shared library dependencies via `ldd` and launcher setup) ✔️
4. Reinstall/update logic ✔️
5. Checksum verification ✔️
6. Corrupted checksum correctly aborts installation ✔️
7. Uninstall (Default vs Purge) ✔️
8. Saves/config preservation during updates and uninstalls ✔️
9. No dependency on `/home/a1` (tested under isolated `/tmp/test_home_bb` mock environment) ✔️
10. No dependency on Unreal Engine for the installed copy (standalone runtime) ✔️

## Results
**VALIDATION PASSED**. 

## Known Limitations
- The current build is a Development build which includes debug symbols (`.debug`, `.sym`) and Vulkan trace/dump layers, totaling ~470MB of non-essential weight. We have opted not to strip these in this initial package to respect the "correctness first, avoid aggressive deletion" directive, but a Shipping configuration can significantly shrink the uncompressed footprint in the future.
- Automated testing verifies library linkages and binary presence but does not launch a full graphical X11/Wayland context headlessly.
