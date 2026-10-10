#!/usr/bin/env python3
import os,pathlib,subprocess,json
root=pathlib.Path(__file__).resolve().parents[1];build=root/'src/threads/build';out=root/'experiments/results'
env=os.environ.copy();env['PATH']=str(build/'qemu-bin')+':'+str(root/'src/utils')+':'+env['PATH']
results=[]
for queue in ('list','rbt'):
 for name in ('alarm-single','alarm-multiple','alarm-simultaneous','alarm-zero','alarm-negative','alarm-priority','priority-fifo','priority-change','priority-preempt','priority-sema','priority-condvar'):
  args=['pintos','--qemu','-v','-T','15','--filesys-size=2','--']+(['-rbt'] if queue=='rbt' else [])+['-f','-q','kernel-test',name]
  p=subprocess.run(args,cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=25)
  output=build/f'tests/threads/{name}.output';output.parent.mkdir(parents=True,exist_ok=True);output.write_text(p.stdout)
  (out/f'{queue}-{name}.log').write_text(p.stdout)
  ck=subprocess.run(['perl','-I../..',f'../../tests/threads/{name}.ck',f'tests/threads/{name}','../..'],cwd=build,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
  resultpath=output.with_suffix('.result')
  result=resultpath.read_text() if resultpath.exists() else ck.stdout
  (out/f'{queue}-{name}.result').write_text(result)
  results.append({'queue':queue,'test':name,'result':result.splitlines()[0] if result else 'ERROR'})
  print(queue,name,results[-1]['result'],flush=True)
(out/'regressions.json').write_text(json.dumps(results,indent=2))
