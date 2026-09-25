"""Only regenerate the service-door clearance and relocate its obstructing console.
Default V0.4/V0.5 generators retain their original reproducible output.
"""
from pathlib import Path
import sys
import importlib.util
ROOT=Path(__file__).resolve().parent
sys.path.insert(0,str(ROOT.parent/'VisualRebuildV04'))
sys.path.insert(0,str(ROOT.parent/'HeroEnvironmentV05'))
from export_obj import write_mesh

def source(name,folder):
    spec=importlib.util.spec_from_file_location(name,ROOT.parent/folder/'generate.py')
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
    return module

if __name__=='__main__':
    out=ROOT/'Generated';out.mkdir(exist_ok=True)
    write_mesh(source('v05','HeroEnvironmentV05').lantern(playtest_access=True),out)
    write_mesh(source('v04','VisualRebuildV04').gallery(playtest_access=True),out)
