"""Offline end-to-end installer tests. Real Bash/Python/zstd/filesystem; fixture HTTP transport."""
import fcntl
import hashlib
import io
import json
import os
from pathlib import Path
import shutil
import subprocess
import tarfile
import tempfile
import time
import unittest

ROOT = Path(__file__).resolve().parents[2]
REPO = 'fixture-owner/BlackBeacon'
URL = f'https://github.com/{REPO}/releases/download/'


class Installer(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory(prefix='bb-installer-test-')
        self.addCleanup(self.temp.cleanup)
        self.work = Path(self.temp.name)
        self.home = self.work / 'tester space'
        self.home.mkdir()
        self.root = self.home / '.local/share/black-beacon'
        self.bin = self.work / 'bin'
        self.bin.mkdir()
        self.transport = self.work / 'transport.json'
        self.log = self.work / 'requests.log'
        self.env = dict(os.environ, HOME=str(self.home),
            PATH=str(self.bin)+os.pathsep+os.environ['PATH'], BLACK_BEACON_REPOSITORY=REPO,
            BB_FIXTURE_TRANSPORT=str(self.transport), BB_FIXTURE_LOG=str(self.log))
        self.assets = {}
        self.releases = []
        curl = self.bin / 'curl'
        curl.write_text('''#!/usr/bin/env python3
import json, os, pathlib, shutil, sys, time
args=sys.argv[1:]; url=args[-1]
with open(os.environ['BB_FIXTURE_LOG'],'a') as f:f.write(url+'\\n')
source=json.load(open(os.environ['BB_FIXTURE_TRANSPORT'])).get(url)
if os.environ.get('BB_FIXTURE_PAUSE')==url:
 pathlib.Path(os.environ['BB_FIXTURE_LOG']+'.paused').touch();time.sleep(30)
if not source:sys.exit(22)
if '--output' in args:
 dest=pathlib.Path(args[args.index('--output')+1])
 if os.environ.get('BB_FIXTURE_FAIL')==url:
  dest.write_bytes(b'partial');sys.exit(18)
 shutil.copyfile(source,dest)
else:sys.stdout.buffer.write(pathlib.Path(source).read_bytes())
''')
        curl.chmod(0o755)

    def package(self, tag='test1-r2', corrupt=False, unsafe=False, real=None):
        name=f'BlackBeacon-linux-x86_64-{tag}.tar.zst'
        archive=self.work/name
        if real:
            archive=Path(real)
            name=archive.name
        else:
            raw=self.work/(tag+'.tar')
            with tarfile.open(raw,'w') as tar:
                files={
                    'black-beacon.sh': b'#!/bin/bash\nSCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"\nexec "$SCRIPT_DIR/BlackBeacon/Binaries/Linux/BlackBeacon" "$@"\n',
                    'BlackBeacon/Binaries/Linux/BlackBeacon': b'#!/bin/bash\nprintf "%s\\n" "$@"\n',
                    'BlackBeacon/Content/version.txt':tag.encode(),
                    'BlackBeacon/Saved/SaveGames/developer.sav': b'must not install developer saves',
                    'Engine/Saved/Config/old.ini': b'must not install developer config'}
                if unsafe:files['../../escaped']=b'bad'
                for path,data in files.items():
                    item=tarfile.TarInfo('package/'+path);item.size=len(data)
                    item.mode=0o755 if path.endswith(('black-beacon.sh','/BlackBeacon')) else 0o644
                    tar.addfile(item,io.BytesIO(data))
            subprocess.run(['zstd','-q','-f',str(raw),'-o',str(archive)],check=True)
        checksum=self.work/(name+'.sha256')
        if real:
            shutil.copyfile(str(real)+'.sha256',checksum)
        else:
            digest=hashlib.sha256(archive.read_bytes()).hexdigest()
            checksum.write_text(('0'*64 if corrupt else digest)+'  '+name+'\n')
        assets=[]
        for filename,source in ((name,archive),(name+'.sha256',checksum)):
            url=URL+tag+'/'+filename
            self.assets[url]=str(source)
            assets.append({'name':filename,'state':'uploaded','browser_download_url':url})
        release={'tag_name':tag,'published_at':f'2026-09-{20+len(self.releases):02d}T10:00:00Z',
                 'draft':False,'prerelease':True,'assets':assets}
        self.releases.append(release)
        self.refresh()
        return release

    def refresh(self):
        metadata=self.work/'releases.json';metadata.write_text(json.dumps(self.releases))
        self.assets[f'https://api.github.com/repos/{REPO}/releases?per_page=100&page=1']=str(metadata)
        self.assets['https://raw.githubusercontent.com/'+REPO+'/main/install.sh']=str(ROOT/'install.sh')
        self.transport.write_text(json.dumps(self.assets))

    def run_script(self, script='install.sh', *args, ok=True):
        result=subprocess.run(['bash',str(ROOT/script),*args],env=self.env,text=True,capture_output=True)
        self.assertEqual(result.returncode==0,ok,result.stdout+'\n'+result.stderr)
        self.assertFalse(list((self.home/'.local/share').glob('.black-beacon-download-*')))
        return result

    def test_pipe_fresh_update_repeat_and_uninstall_preserve_data(self):
        self.package()
        result=subprocess.run(['bash','-o','pipefail','-c',
            f'curl -fsSL https://raw.githubusercontent.com/{REPO}/main/install.sh | bash'],
            env=self.env,text=True,capture_output=True)
        self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        self.assertFalse((self.root/'current/BlackBeacon/Saved/SaveGames/developer.sav').exists())
        save=self.root/'current/BlackBeacon/Saved/SaveGames/owner.sav'
        save.parent.mkdir();save.write_text('keep me')
        config=self.root/'current/Engine/Saved/custom.ini';config.write_text('settings')
        launch=subprocess.run([str(self.home/'.local/bin/black-beacon'),'space argument','--flag'],
                              env=self.env,text=True,capture_output=True)
        self.assertEqual(launch.returncode,0,launch.stderr)
        self.assertEqual(launch.stdout.splitlines(),['space argument','--flag'])
        self.package('test2')
        self.run_script()
        self.assertEqual(save.read_text(),'keep me')
        self.assertEqual(config.read_text(),'settings')
        self.assertEqual(len(list((self.root/'versions').iterdir())),1)
        before=self.log.read_text().count('.tar.zst\n')
        self.run_script()
        self.assertEqual(self.log.read_text().count('.tar.zst\n'),before)
        native=self.home/'.config/Epic/BlackBeacon/Saved/native.sav'
        native.parent.mkdir(parents=True);native.write_text('native')
        self.run_script('uninstall-black-beacon.sh')
        self.assertFalse((self.root/'current').exists())
        self.assertFalse((self.home/'.local/bin/black-beacon').exists())
        self.assertEqual((self.root/'userdata/BlackBeacon/Saved/SaveGames/owner.sav').read_text(),'keep me')
        self.run_script()
        self.assertEqual(save.read_text(),'keep me')
        self.run_script('uninstall-black-beacon.sh','--remove-saves')
        self.assertFalse((self.root/'userdata').exists())
        self.assertEqual(native.read_text(),'native')

    def test_bad_checksum_and_partial_download_leave_current_untouched(self):
        self.package();self.run_script();before=os.readlink(self.root/'current')
        release=self.package('broken',corrupt=True)
        result=self.run_script(ok=False)
        self.assertIn('SHA-256 mismatch',result.stderr)
        self.assertEqual(os.readlink(self.root/'current'),before)
        self.env['BB_FIXTURE_FAIL']=release['assets'][0]['browser_download_url']
        self.run_script(ok=False)
        self.assertEqual(os.readlink(self.root/'current'),before)

    def test_termination_cleans_downloads_and_keeps_current_version(self):
        self.package();self.run_script();before=os.readlink(self.root/'current')
        release=self.package('test2')
        self.env['BB_FIXTURE_PAUSE']=release['assets'][0]['browser_download_url']
        process=subprocess.Popen(['bash',str(ROOT/'install.sh')],env=self.env,
                                 stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
        try:
            for _ in range(100):
                if Path(str(self.log)+'.paused').exists():break
                time.sleep(.05)
            else:self.fail('Download did not reach the interruption fixture')
            process.terminate()
            out,err=process.communicate(timeout=10)
            self.assertNotEqual(process.returncode,0,out+err)
            self.assertIn('Interrupted',err)
            self.assertEqual(os.readlink(self.root/'current'),before)
            self.assertFalse(list((self.home/'.local/share').glob('.black-beacon-download-*')))
        finally:
            if process.poll() is None:process.kill();process.communicate()

    def test_unsafe_verified_archive_is_rejected(self):
        self.package(unsafe=True)
        result=self.run_script(ok=False)
        self.assertIn('Unsafe archive',result.stderr)
        self.assertFalse((self.home/'.local/share/escaped').exists())
        self.assertFalse((self.root/'current').exists())

    def test_missing_release_pair_and_missing_repository_fail_clearly(self):
        release=self.package();release['assets'].pop();self.refresh()
        self.assertIn('No published Linux',self.run_script(ok=False).stderr)
        self.env['BLACK_BEACON_REPOSITORY']='OWNER/REPO'
        self.assertIn('not configured',self.run_script(ok=False).stderr)

    def test_running_game_lock_blocks_update_and_uninstall(self):
        self.package();self.run_script()
        with (self.home/'.local/share/.black-beacon-install.lock').open('a') as lock:
            fcntl.flock(lock,fcntl.LOCK_SH)
            self.assertIn('running',self.run_script(ok=False).stderr)
            self.assertIn('running',self.run_script('uninstall-black-beacon.sh',ok=False).stderr)

    def test_legacy_install_migrates_both_saved_trees(self):
        self.root.mkdir(parents=True)
        (self.root/'black-beacon.sh').write_text('# black-beacon legacy launcher')
        for relative in ('BlackBeacon/Saved/owner.sav','Engine/Saved/custom.ini'):
            path=self.root/relative;path.parent.mkdir(parents=True);path.write_text(relative)
        (self.root/'personal.txt').write_text('do not delete unknown files')
        self.package();self.run_script()
        for relative in ('BlackBeacon/Saved/owner.sav','Engine/Saved/custom.ini'):
            self.assertEqual((self.root/'current'/relative).read_text(),relative)
        self.assertFalse((self.root/'black-beacon.sh').exists())
        self.assertTrue((self.root/'personal.txt').exists())

    def test_newest_published_linux_includes_prereleases_and_skips_other_platforms(self):
        first=self.package('older-tag');second=self.package('newer-tag')
        first['published_at']='2026-12-01T00:00:00Z'
        self.releases.append(dict(second,tag_name='windows-only',published_at='2027-01-01T00:00:00Z',assets=[]))
        self.refresh()
        self.assertIn('older-tag',self.run_script().stdout)

    def test_unrecognized_install_and_uninstall_arguments_do_not_delete(self):
        self.root.mkdir(parents=True);sentinel=self.root/'valuable.txt';sentinel.write_text('keep')
        self.package();self.run_script(ok=False)
        self.run_script('uninstall-black-beacon.sh',ok=False)
        self.run_script('uninstall-black-beacon.sh','--typo',ok=False)
        self.assertEqual(sentinel.read_text(),'keep')

    @unittest.skipIf(os.geteuid() == 0, 'Permission-denied rollback needs an unprivileged user')
    def test_failed_launcher_transaction_rolls_back_application(self):
        self.package();self.run_script()
        before=os.readlink(self.root/'current')
        launcher=(self.home/'.local/bin/black-beacon').read_bytes()
        self.package('test2')
        apps=self.home/'.local/share/applications'
        apps.chmod(0o500)
        try:
            self.run_script(ok=False)
            self.assertEqual(os.readlink(self.root/'current'),before)
            self.assertEqual((self.home/'.local/bin/black-beacon').read_bytes(),launcher)
            self.assertEqual(len(list((self.root/'versions').iterdir())),1)
        finally:
            apps.chmod(0o700)

    def test_desktop_with_special_home_characters_is_valid(self):
        self.home=self.work/'name with "quotes" $dollar `tick` %field'
        self.home.mkdir()
        self.root=self.home/'.local/share/black-beacon'
        self.env['HOME']=str(self.home)
        self.package();self.run_script()
        desktop=self.home/'.local/share/applications/black-beacon.desktop'
        if shutil.which('desktop-file-validate'):
            result=subprocess.run(['desktop-file-validate',str(desktop)],text=True,capture_output=True)
            self.assertEqual(result.returncode,0,result.stdout+result.stderr)
        launch=subprocess.run([str(self.home/'.local/bin/black-beacon'),'argument'],env=self.env,text=True,capture_output=True)
        self.assertEqual(launch.stdout,'argument\n',launch.stderr)

    def test_unsupported_architecture_and_missing_dependency(self):
        self.package()
        uname=self.bin/'uname'
        uname.write_text('#!/bin/bash\nif [[ "$1" == -m ]]; then echo aarch64; else echo Linux; fi\n')
        uname.chmod(0o755)
        self.assertIn('Linux x86_64',self.run_script(ok=False).stderr)
        uname.unlink()
        for tool in ('bash','uname','python3'):
            (self.bin/tool).symlink_to(shutil.which(tool))
        self.env['PATH']=str(self.bin)
        self.assertIn('Missing required tool: zstd',self.run_script(ok=False).stderr)

    @unittest.skipUnless(os.environ.get('BB_TEST_REAL_ARCHIVE'),'Optional existing-package extraction')
    def test_existing_validated_archive_without_repackaging_or_game_launch(self):
        self.package(real=os.environ['BB_TEST_REAL_ARCHIVE'])
        self.run_script()
        self.assertTrue((self.root/'current/BlackBeacon/Binaries/Linux/BlackBeacon').is_file())
        self.assertFalse(any((self.root/'userdata/BlackBeacon/Saved').iterdir()))
        self.run_script('uninstall-black-beacon.sh')


if __name__=='__main__':unittest.main(verbosity=2)
