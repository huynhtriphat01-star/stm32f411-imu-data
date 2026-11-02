"""Audit existing LAB01 CSVs; never modify measurements or infer capture metadata."""
from pathlib import Path
import argparse, csv, hashlib, json
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]

def audit(path, output):
    with path.open(encoding='utf-8-sig', newline='') as f:
        rows = list(csv.DictReader(f))
    if len(rows) < 2:
        raise ValueError(f'{path}: at least two samples required')
    seq = np.array([int(r['seq']) for r in rows], dtype=np.int64)
    ts = np.array([int(r['timestamp_us']) for r in rows], dtype=np.int64)
    raw = np.array([[float(r[k+'_raw']) for k in ['ax','ay','az','gx','gy','gz']] for r in rows])
    a = np.array([[float(r[k+'_g']) for k in ['ax','ay','az']] for r in rows])
    g = np.array([[float(r[k+'_dps']) for k in ['gx','gy','gz']] for r in rows])
    if not all(np.isfinite(x).all() for x in [raw,a,g]):
        raise ValueError(f'{path}: nonfinite numeric data')
    label, session = path.parent.name, path.stem.removeprefix('session_')
    if any(r['label'] != label or r['session'] != session for r in rows):
        raise ValueError(f'{path}: label/session mismatch')
    dt = np.diff(ts) % 2**32
    steps = np.diff(seq) % 65536
    # Backward jumps / resets must not be interpreted as tens of thousands of lost frames.
    ambiguous = bool(np.any((steps == 0) | (steps >= 32768)) or np.any(dt == 0))
    lost = int(np.sum(steps-1)) if not ambiguous else None
    loss = 100*lost/(len(rows)+lost) if lost is not None else None
    norm = np.linalg.norm(a,axis=1)
    result = dict(file=path.relative_to(ROOT).as_posix(),samples=len(rows), label=label,session=session,
                  duration_s=float(dt.sum()/1e6),rate_hz=float(1e6/np.mean(dt)),
                  sequence_ambiguous=ambiguous,lost_frames=lost,loss_percent=loss,
                  jitter_p99_us=float(np.percentile(np.abs(dt-10000),99)),
                  acceleration_norm_mean_g=float(norm.mean()),
                  gyro_mean_dps=g.mean(axis=0).tolist(),gyro_sd_dps=g.std(axis=0,ddof=1).tolist(),
                  saturated_samples=int(np.any((raw<=-32768)|(raw>=32767),axis=1).sum()),
                  crc_errors=None,subject=None,position=None,
                  metadata_provenance='Retrospective audit; capture-time metadata unavailable',
                  sha256=hashlib.sha256(path.read_bytes()).hexdigest())
    result['timing_sequence_pass'] = bool(not ambiguous and loss < .5 and result['jitter_p99_us'] < 1000)
    result['idle_gravity_pass'] = bool(.95 <= norm.mean() <= 1.05) if label=='idle' else None
    stem=f'{label}_{session}'
    output.mkdir(parents=True,exist_ok=True)
    (output/(stem+'.json')).write_text(json.dumps(result,indent=2,ensure_ascii=False),encoding='utf-8')
    t=np.r_[0,np.cumsum(dt)]/1e6
    fig, axes=plt.subplots(3,1,figsize=(10,7),layout='constrained')
    for i,name in enumerate('XYZ'):
        axes[0].plot(t,a[:,i],label=name,linewidth=.6,linestyle=['-','--',':'][i])
    axes[0].set(ylabel='Acceleration (g)',title=stem+' — original samples'); axes[0].legend(ncol=3)
    axes[1].plot(t,norm,color='black',linewidth=.6); axes[1].set(ylabel='|a| (g)')
    axes[2].plot(t[1:],dt-10000,color='#0072B2',linewidth=.6)
    axes[2].set(xlabel='MCU elapsed time (s)',ylabel='dt - 10000 (us)')
    fig.savefig(output/(stem+'_quality.png'),dpi=160); plt.close(fig)
    return result

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__); parser.add_argument('file',nargs='?',type=Path)
    args=parser.parse_args(); files=[args.file.resolve()] if args.file else sorted((ROOT/'raw').glob('*/*.csv'))
    results=[audit(f,ROOT/'docs/quality') for f in files]
    fields=['file','samples','duration_s','rate_hz','lost_frames','loss_percent','jitter_p99_us','acceleration_norm_mean_g','saturated_samples','timing_sequence_pass','idle_gravity_pass','crc_errors']
    with (ROOT/'docs/quality/summary.csv').open('w',newline='',encoding='utf-8') as f:
        w=csv.DictWriter(f,fieldnames=fields,extrasaction='ignore'); w.writeheader(); w.writerows(results)
    print(json.dumps(results,ensure_ascii=False))
