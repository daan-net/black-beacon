# Asset provenance — environment kit v1

All new assets come directly from the named creator's official Poly Haven page.
[Poly Haven's asset license](https://polyhaven.com/license) permits commercial
use and redistribution under [CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/);
attribution is not required. Credit is retained here. No ripped commercial-game
content, marketplace mirrors, or unclear uploader claims are used.

| Asset | Source URL / creator | License / attribution | Black Beacon use |
| --- | --- | --- | --- |
| Painted Plaster Wall | https://polyhaven.com/a/painted_plaster_wall · Amal Kumar | CC0-1.0; not required | Kit wall material, lighthouse exterior/interior and annex wall slots. |
| Coast Rocks 05 | https://polyhaven.com/a/coast_rocks_05 · Rob Tuytel | CC0-1.0; not required | Noncolliding immediate coastal ledges around lighthouse. |
| Rock Face 02 | https://polyhaven.com/a/rock_face_02 · Dario Barresi; Rico Cilliers (processing) | CC0-1.0; not required | Noncolliding near-coast rock-face dressing. |
| Barrel 03 | https://polyhaven.com/a/barrel_03 · Serhii Khromov | CC0-1.0; not required | Annex fuel-storage dressing; no interaction or collision. |

Acquisition record: `Art/Source/EnvironmentKitV1/assets.json` pins exact download
URLs, provider MD5, locally verified SHA-256, physical dimensions and authors.
`acquire.py` verifies originals before use. Original downloads are reproducibly
cached under ignored `Saved/EnvironmentKitV1/Sources`; imported runtime assets
live in `Content/BlackBeacon/EnvironmentKitV1`. No preview renders or website
copy are shipped as game textures. Source UVs are retained on imported props.

## Existing content retained

The historical [root provenance record](../ASSET_PROVENANCE.md) remains intact:
the wreck source is documented there as CC0, and earlier ImageGen albedos are
project-generated. Engine sample boulders and native cloud resources remain under
their existing Unreal content terms, not CC0. The original Python/Blender
lighthouse, generator, stairs, ocean and storm audio sources are retained.
This pass does not relicense or replace those assets.

## Not introduced

Fab Nordic Coastal Cliff and the later pipe kit are acquisition candidates only.
No license entitlement, paid purchase, download or use is claimed for them here.
No The Last of Us / Naughty Dog or other ripped game assets are included.
