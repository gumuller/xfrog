#!/usr/bin/env python3
"""Analyze original courier drawings; write displacement data, never artwork.

Authoring only: pip install opencv-python-headless==5.0.0.93 numpy==2.5.4
The game uses the checked-in motion data and has no OpenCV/Python dependency.
"""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import cv2
import numpy as np

GRID, CANVAS, HEIGHT, BASE = 64, 256, 200, 230
ORDERS = [[0, 1, 0, 1, 0, 1], [0, 2, 1, 3, 6, 4], [0, 1, 0, 1, 0, 1]]

def analyze(path):
    rgba = cv2.imread(str(path), cv2.IMREAD_UNCHANGED)
    count, labels, stats, _ = cv2.connectedComponentsWithStats((rgba[:, :, 3] > 32).astype(np.uint8), 8)
    ids = sorted(range(1, count), key=lambda i: -stats[i, 4])[:32]
    rgba[~np.isin(labels, ids)] = 0
    ids.sort(key=lambda i: 2 * stats[i, 1] + stats[i, 3])
    ids = sum([sorted(ids[i:i+8], key=lambda j: stats[j, 0]) for i in range(0, 32, 8)], [])
    frames = []
    yy, xx = np.mgrid[:CANVAS, :CANVAS].astype(np.float32)
    for row in range(3):
        ref = max(stats[i, 3] for i in ids[row*8:row*8+8])
        sequence = []
        feet = []
        for key in ORDERS[row]:
            i = ids[row*8+key]
            x, y, w, h, _ = stats[i]
            mask = (labels[y:y+int(h*.34), x:x+w] == i)
            weights = mask * (rgba[y:y+int(h*.34), x:x+w, 3] / 255.)
            anchor = x + (weights * np.arange(w)).sum() / weights.sum()
            sx = (xx - CANVAS/2) * ref / HEIGHT + anchor
            sy = (yy - BASE) * ref / HEIGHT + y+h
            sampled = cv2.remap(rgba, sx.astype(np.float32), sy.astype(np.float32), cv2.INTER_LINEAR, borderMode=cv2.BORDER_CONSTANT)
            outside = (sx < x) | (sx >= x+w) | (sy < y) | (sy >= y+h)
            sampled[outside] = 0
            alpha = sampled[:, :, 3].astype(np.float32)/255
            gray = cv2.cvtColor(sampled[:, :, :3], cv2.COLOR_BGR2GRAY).astype(np.float32)
            # Alpha makes even dark trousers distinct from the neutral canvas.
            hsv = cv2.cvtColor(sampled[:, :, :3], cv2.COLOR_BGR2HSV)
            shoes = ((hsv[:,:,0] < 16) | (hsv[:,:,0] > 170)) & (hsv[:,:,1] > 80) & (hsv[:,:,2] > 75) & (sampled[:,:,3] > 80) & (yy > 157)
            n, ls, st, cent = cv2.connectedComponentsWithStats(shoes.astype(np.uint8),8)
            largest = sorted((k for k in range(1,n) if st[k,1]+st[k,3] > (185 if row == 1 else 175)), key=lambda k:-st[k,4])[:2]
            shoe_points = sorted([cent[k] for k in largest],key=lambda p:p[0])
            if len(shoe_points)!=2: raise ValueError(f'Missing shoes {path} {row} {key}')
            feet.append(np.array(shoe_points))
            sequence.append(np.uint8(np.clip(gray * alpha * .7 + 80 * alpha + 12*(1-alpha), 0, 255)))
        frames.append((sequence,feet))
    cv2.setNumThreads(1)
    flow = cv2.DISOpticalFlow_create(cv2.DISOPTICAL_FLOW_PRESET_MEDIUM)
    flow.setFinestScale(0)
    flow.setVariationalRefinementIterations(12)
    result = bytearray(b'XFMOTION' + struct.pack('<5I', GRID, 6, 3, HEIGHT, BASE))
    for sequence, feet in frames:
        for key in range(6):
            a, b = sequence[key], sequence[(key+1) % 6]
            for first, last, start, end in [(a,b,feet[key],feet[(key+1)%6]), (b,a,feet[(key+1)%6],feet[key])]:
                initial = np.zeros((CANVAS,CANVAS,2),np.float32)
                total = 3*np.exp(-((yy-110)/40)**2/2) + .02
                for point, delta in zip(start,end-start):
                    weight=np.exp(-((xx-point[0])**2+(yy-point[1])**2)/(2*32**2))
                    initial += weight[:,:,None]*delta
                    total += weight
                initial /= total[:,:,None]
                field = flow.calc(first, last, initial)
                # Keep the red shoe landmarks attached while the dense solver
                # refines clothing/face correspondence in low-texture areas.
                for point, delta in zip(start,end-start):
                    px,py = np.rint(point).astype(int)
                    correction=delta-field[py,px]
                    weight=np.exp(-((xx-point[0])**2+(yy-point[1])**2)/(2*22**2))
                    field += weight[:,:,None]*correction
                field = cv2.GaussianBlur(field, (5,5), .8)
                # Nodes include the full canvas endpoints for bilinear sampling.
                axis = np.linspace(0, CANVAS-1, GRID, dtype=np.float32)
                mx,my = np.meshgrid(axis,axis)
                field = cv2.remap(field,mx,my,cv2.INTER_LINEAR)
                result.extend(np.rint(np.clip(field,-96,96)*256).astype('<i2').tobytes())
    target = path.with_suffix('.motion')
    target.write_bytes(result)
    return target

if __name__ == '__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('assets',type=Path)
    args=parser.parse_args()
    manifest_path=args.assets/'manifest.json'
    manifest=json.loads(manifest_path.read_text())
    for name in ['courier.png','carry-walk.png']:
        target=analyze(args.assets/name)
        manifest['files'][target.name]={'sha256':hashlib.sha256(target.read_bytes()).hexdigest(), 'source_sha256':manifest['files'][name]['sha256']}
        print(target, target.stat().st_size)
    manifest_path.write_text(json.dumps(manifest,indent=2)+'\n')
