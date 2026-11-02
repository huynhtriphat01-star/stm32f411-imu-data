"""Compute a Hann-window periodogram of existing gesture recordings (G4 only)."""
from pathlib import Path
import csv,json
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

ROOT=Path(__file__).resolve().parents[1]
out=ROOT/'docs/psd'; out.mkdir(parents=True,exist_ok=True)
results=[]
for path in sorted((ROOT/'raw').glob('*/*.csv')):
    with path.open(encoding='utf-8-sig') as f: rows=list(csv.DictReader(f))
    ts=np.array([int(r['timestamp_us']) for r in rows],dtype=np.int64)
    dt=np.diff(ts)%2**32
    if not np.all(dt==10000): raise ValueError(f'{path}: uniform 100 Hz samples required')
    fs=1e6/dt.mean(); n=len(rows); window=np.hanning(n); freq=np.fft.rfftfreq(n,1/fs)
    fig,axes=plt.subplots(2,1,figsize=(9,6),layout='constrained')
    record={'file':path.relative_to(ROOT).as_posix(),'method':'Hann periodogram, subtract axis mean, one-sided PSD','axes':{}}
    for axis,cols,unit in [(axes[0],['ax_g','ay_g','az_g'],'g²/Hz'),(axes[1],['gx_dps','gy_dps','gz_dps'],'(deg/s)²/Hz')]:
        for i,col in enumerate(cols):
            x=np.array([float(r[col]) for r in rows]); power=np.abs(np.fft.rfft((x-x.mean())*window))**2/(fs*(window**2).sum())
            if n%2==0: power[1:-1]*=2
            else: power[1:]*=2
            mask=(freq>=.5)&(freq<=20)
            peak=float(freq[mask][np.argmax(power[mask])]); record['axes'][col]={'peak_0_5_to_20_hz':peak,'fraction_power_below_20_hz':float(power[freq<=20].sum()/power.sum()) if power.sum() else None}
            axis.semilogy(freq,np.maximum(power,np.finfo(float).tiny),label=col,linestyle=['-','--',':'][i],linewidth=.8)
        axis.set(xlim=(0,50),ylabel=unit); axis.legend(ncol=3)
    axes[0].set_title(path.parent.name+'/'+path.name+' — no temporal filtering')
    axes[1].set_xlabel('Frequency (Hz)'); fig.savefig(out/(path.parent.name+'_'+path.stem+'.png'),dpi=160); plt.close(fig)
    results.append(record)
(out/'summary.json').write_text(json.dumps(results,indent=2),encoding='utf-8')
print('PSD files:',len(results))
