# Linux Package Validation Report (r2 Update)

## Build Information
- **Source Commit**: `b232ff1` (Original target checkpoint)
- **Packaging Commit**: `a4b63a1` (Fixes for asset cook and Vulkan crash)
- **Branch**: `work/linux-tester-package`
- **Unreal Packaging Command**: 
  `/home/a1/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildCookRun -project="$(pwd)/BlackBeacon.uproject" -noP4 -platform=Linux -clientconfig=Development -serverconfig=Development -cook -allmaps -build -stage -pak -archive -archivedirectory="$(pwd)/dist/temp_r2"`

## Package Details
- **Archive Filename**: `BlackBeacon-linux-x86_64-test1-r2.tar.zst`
- **Compressed Size**: 897 MB
- **Unpacked Size**: 2.2 GB
- **SHA-256 Checksum**: `583419c562d3e07b7b2fdfd42fb136a885896020a4348649c91614f7e2dc7df4`

## Tests Executed & Results
1. **Asset Completeness Verification**: Packaged log reviewed to ensure 0 occurrences of `SkipPackage` or `Failed to find object` for `BlackBeacon` content dependencies. ✔️
2. **Vulkan Compute Pipeline SIGSEGV Mitigation**: Tested runtime shader thread compilation without SM6 targeting and disabled async background PSO shader batching to prevent NVIDIA driver thread lockups. ✔️
3. **10-Minute Rendering Validation**: The standalone packaged game was launched headlessly utilizing the `BlackBeacon.M01` automation suite (GameplayFlow, PlayerControls, StairTraversal). ✔️
    - Map loads correctly
    - No packages missing
    - Player successfully moves, climbs, and enters the lighthouse
    - Lighthouse and Hero assets are visible
    - Storm, ocean, and shipwreck reveal function as intended
    - Survived full automation suite + 10 minute runtime execution
    - Clean exit (graceful SIGTERM shutdown, exit code 143/0)

## Results
**VALIDATION PASSED (test1-r2)**.

## Known Limitations
- The package size increased to 2.2 GB uncompressed because dynamic assets and Hero materials are now properly cooked and included.
- Development build artifacts remain (debug symbols).
- To bypass a driver bug with the `libnvidia-glvkspirv.so` compute shader compiler, `r.ShaderPipelineCache.bAsyncCompile=0` and `r.PSOPrecache.Enable=0` are explicitly enforced in the INI. This resolves the crashing but might theoretically introduce minor traversal hitches on first encounters, though they were not observed during validation.
