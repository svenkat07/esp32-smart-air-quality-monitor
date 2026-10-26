"""Build a dependency-free report from actual host-simulator output (synthetic data)."""
import json
from pathlib import Path
from collections import Counter

root = Path(__file__).resolve().parents[1]
rows = [json.loads(x) for x in (root/'examples/simulated_telemetry.jsonl').read_text().splitlines()]
assert len(rows) == 120 and all(x['simulated'] is True for x in rows)
assert {x['state'] for x in rows} == {'normal','warmup','alert','sensor_fault'}

def plot(key, label, low, high):
    # Separate temperature path segments at missing readings instead of bridging gaps.
    paths=[];segment=[]
    for i,r in enumerate(rows):
        if r[key] is None:
            if segment: paths.append(' '.join(segment));segment=[]
        else:segment.append(f'{50+i/119*800:.1f},{180-(r[key]-low)/(high-low)*140:.1f}')
    if segment:paths.append(' '.join(segment))
    lines=''.join(f'<polyline points="{p}" fill="none" stroke="#146c80" stroke-width="2"/>' for p in paths)
    return f'<h2>{label}</h2><svg viewBox="0 0 900 220" role="img" aria-label="Simulated {label}"><path d="M50 25V180H850" fill="none" stroke="#888"/><text x="5" y="35">{high}</text><text x="5" y="180">{low}</text>{lines}<text x="50" y="210">0 s</text><text x="795" y="210">238 s</text></svg>'

counts=Counter(r['state'] for r in rows)
html='''<!doctype html><html lang="en"><meta charset="utf-8"><title>Simulated ESP32 monitor</title>
<style>body{max-width:960px;margin:40px auto;padding:0 22px;font:16px/1.6 system-ui;color:#163343;background:#f6f8fa}header,section{padding:24px;background:white;border:1px solid #d6e2e8;border-radius:12px;margin:20px 0}svg{width:100%;background:#fafcfd}strong{color:#ad4512}h1{line-height:1.2}code{background:#edf2f4}</style>
<header><strong>SIMULATED DATA • SOFTWARE-ONLY RECONSTRUCTION</strong><h1>ESP32 environmental monitor</h1><p>Deterministic host simulation, 120 samples over 238 logical seconds. No physical measurements or live cloud connection.</p></header><section>'''
html+=f'<p>State counts: {dict(counts)}</p>'
html+=plot('gas_filtered_mv','Filtered analogue voltage (mV), synthetic',0,2500)
html+=plot('temperature_c','Temperature (°C), synthetic; gap = sensor fault',0,45)
html+='''<p>0–58 s: application startup gate. 80–128 s: synthetic analogue excursion.
150–168 s: synthetic temperature excursion. 180–186 s: invalid temperature values.
State changes can lag the analogue excursion because smoothing and hysteresis are enabled.</p>
<p>Voltage is not CO2 ppm or AQI. Thresholds are demonstration parameters.</p></section></html>'''
(root/'examples/simulation_report.html').write_text(html,encoding='utf-8')
print('Report created; counts:', dict(counts))
