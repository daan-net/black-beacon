"""Download the approved small CC0 batch; cache originals outside Git, pin checksums."""
import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[3]
SOURCE = Path(__file__).resolve().parent
CACHE = ROOT / 'Saved/EnvironmentKitV1/Sources'
SLUGS = ('painted_plaster_wall', 'coast_rocks_05', 'rock_face_02', 'barrel_03')
manifest_path = SOURCE / 'assets.json'


def fetch(url, path):
    path.parent.mkdir(parents=True, exist_ok=True)
    subprocess.run(['curl', '-fSL', '--retry', '2', '--connect-timeout', '20',
                    '--max-time', '600', '-o', str(path), url], check=True)


if manifest_path.exists():
    manifest = json.loads(manifest_path.read_text())
else:
    manifest = []
    for slug in SLUGS:
        info = json.loads((ROOT / f'Saved/EnvironmentKitV1/{slug}-info.json').read_text())
        files = json.loads((ROOT / f'Saved/EnvironmentKitV1/{slug}-files.json').read_text())
        entry = dict(id=slug, name=info['name'], authors=info['authors'],
                     source=f'https://polyhaven.com/a/{slug}', license='CC0-1.0',
                     license_url='https://polyhaven.com/license', attribution_required=False,
                     dimensions_mm=info['dimensions'], files={})
        for channel in ('Diffuse', 'nor_dx', 'arm'):
            entry['files'][channel] = dict(files[channel]['2k']['png'])
        if 'fbx' in files:
            entry['files']['mesh'] = {k: v for k, v in files['fbx']['2k']['fbx'].items() if k != 'include'}
        manifest.append(entry)

for asset in manifest:
    for channel, file in asset['files'].items():
        destination = CACHE / asset['id'] / Path(file['url']).name
        if not destination.exists():
            fetch(file['url'], destination)
        data = destination.read_bytes()
        assert len(data) == file['size'], destination
        assert hashlib.md5(data).hexdigest() == file['md5'], destination
        digest = hashlib.sha256(data).hexdigest()
        if 'sha256' in file:
            assert file['sha256'] == digest, destination
        file['sha256'] = digest
        file['local'] = str(destination.relative_to(ROOT))
    print('Verified CC0 source:', asset['name'], flush=True)
manifest_path.write_text(json.dumps(manifest, indent=2) + '\n')
