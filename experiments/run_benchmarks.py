#!/usr/bin/env python3
import os, pathlib, subprocess, json, re
root=pathlib.Path(__file__).resolve().parents[1]
build=root/'src/threads/build'
out=root/'experiments/results';out.mkdir(exist_ok=True)
env=os.environ.copy();env['PATH']=str(build/'qemu-bin')+':'+str(root/'src/utils')+':'+env['PATH']
rows=[]
for run in range(1,11):
 for queue in ('list','rbt'):
  args=['pintos','--qemu','-v','-T','60','--filesys-size=2','--']
  if queue=='rbt':args+=['-rbt']
  args+=['-f','-q','kernel-test','ready-benchmark']
  proc=subprocess.run(args,cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=70)
  (out/f'{queue}-{run:02}.log').write_text(proc.stdout)
  if proc.returncode or 'BENCH correctness=PASS' not in proc.stdout:raise RuntimeError(proc.stdout)
  for m in re.finditer(r'BENCH queue=(\w+) threads=(\d+) yields=(\d+) inserts=(\d+) comparisons=(\d+) events=(\d+) ticks=(\d+)',proc.stdout):
   row=dict(zip(('queue','threads','yields','inserts','comparisons','events','ticks'),m.groups()))
   row={k:v if k=='queue' else int(v) for k,v in row.items()};row['run']=run;rows.append(row)
  print(f'{queue} repeat {run}: PASS',flush=True)
(out/'measurements.json').write_text(json.dumps(rows,indent=2))
