#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-2.0-or-later
"""Render an original 80-bar, sample-style town score. Requires numpy only to regenerate.
No recordings, soundfonts or third-party melodies. Runtime needs only the WAV.
"""
import argparse
from pathlib import Path
import wave
import numpy as np
RATE = 32000
BPM = 104
BEAT = 60 / BPM
BARS = 80
DURATION = BARS * 4 * BEAT
rng = np.random.default_rng(2048)
mix = np.zeros((int(DURATION * RATE), 2), np.float32)

def note(beat, length, midi, instrument, volume=.07, pan=0):
    start = int(beat * BEAT * RATE)
    duration = length * BEAT
    n = min(int((duration + .25) * RATE), len(mix) - start)
    if n <= 0: return
    t = np.arange(n, dtype=np.float32) / RATE
    hz = 440 * 2 ** ((midi - 69) / 12)
    phase = 2 * np.pi * hz * t
    if instrument == 'flute':
        phase += .22 * np.sin(2*np.pi*4.8*t) * np.minimum(1,t/.2)
        tone = np.sin(phase) + .12*np.sin(2*phase) + .035*np.sin(3*phase)
        attack, decay = .055, .22
        body = np.ones(n)
    elif instrument == 'strings':
        tone = sum(np.sin(phase * (1+detune) + .04*np.sin(2*np.pi*4*t)) * gain
                   for detune,gain in [(-.002, .46),(.002,.46),(.001, .1)])
        tone += .08*np.sin(phase*2)
        attack, decay = .25, .35
        body = np.ones(n)
    elif instrument == 'piano':
        tone = np.sin(phase)*np.exp(-t*2.0) + .27*np.sin(phase*2)*np.exp(-t*4.7) + .08*np.sin(phase*3)*np.exp(-t*8)
        attack, decay = .008, .22
        body = np.ones(n)
    elif instrument == 'vibes':
        tone = np.sin(phase)*np.exp(-t*1.7) + .14*np.sin(phase*3.99)*np.exp(-t*5)
        attack, decay = .006, .3
        body = 1 + .07*np.sin(2*np.pi*5.2*t)
    elif instrument == 'bass':
        tone = np.sin(phase) + .14*np.sin(phase*2)*np.exp(-t*4)
        attack, decay = .018, .12
        body = np.exp(-t*1.5)
    else:
        tone = np.sin(phase) + .23*np.sin(phase*2) + .09*np.sin(phase*3)
        attack, decay = .006, .18
        body = np.exp(-t*5)
    env = np.minimum(t/attack,1) * np.minimum(np.maximum(0,duration+.15-t)/decay,1)
    data = (tone * env * body * volume).astype(np.float32)
    mix[start:start+n,0] += data * np.sqrt((1-pan)/2)
    mix[start:start+n,1] += data * np.sqrt((1+pan)/2)

def drum(beat, kind, volume):
    start=int(beat*BEAT*RATE); n=min(int(.22*RATE),len(mix)-start)
    if n<=0:return
    t=np.arange(n,dtype=np.float32)/RATE
    if kind=='kick':
        phase=2*np.pi*(48*t + 65*(1-np.exp(-t*35))/35)
        sound=np.sin(phase)*np.exp(-t*23)
    elif kind=='snare':
        noise=rng.normal(0,.45,n).astype(np.float32)
        noise=np.convolve(noise,np.ones(5)/5,'same')
        sound=(noise*.8 + np.sin(2*np.pi*170*t)*.16)*np.exp(-t*28)
    else:
        noise=rng.normal(0,.35,n).astype(np.float32)
        sound=(noise-np.roll(noise,1))*.4*np.exp(-t*100)
    mix[start:start+n] += sound[:,None]*volume

# C major / relative minor, contrasting 16-bar phrases and a quiet bridge.
progressions = [
 [(60,4),(67,4),(57,3),(65,4),(62,3),(67,4),(60,4),(67,4)],
 [(65,4),(67,4),(64,3),(57,3),(62,3),(67,4),(60,4),(60,4)],
 [(57,3),(64,3),(65,4),(60,4),(62,3),(57,3),(65,4),(67,4)],
 [(65,4),(60,4),(62,3),(67,4),(64,3),(57,3),(62,3),(67,4)]]
# Explicit original motifs, silence and held notes give the tune room to breathe.
motifs = [
 [(0,1,76),(1,.5,79),(1.5,.5,81),(2,1.5,79)],
 [(0,1.5,74),(2,.5,76),(2.5,.5,74),(3,1,71)],
 [(.5,.5,72),(1,.5,76),(2,1,79),(3,.75,76)],
 [(0,2,77),(2.5,.5,76),(3,1,74)],
 [(0,1,81),(1.5,.5,79),(2,1.5,76)],
 [(.5,1,74),(2,.5,72),(3,.75,71)],
 [(0,1.5,72),(2,1,76),(3,.75,74)],
 [(0,2.75,72)] ]
bridge=[[(0,2,69),(2.5,1,72)],[(0,3,71)],[(.5,1.5,74),(2.5,1,72)],[(0,3,67)]]
for bar in range(BARS):
    if bar<8: section=0
    elif bar<24: section=1
    elif bar<40: section=2
    elif bar<48: section=3
    elif bar<64: section=4
    else: section=5
    prog=progressions[0 if section in (0,1,4) else 1 if section==2 else 2 if section==3 else 3]
    root,third=prog[bar%8]
    base=bar*4
    # Sparse introductory texture, later voicings and countermelodies vary.
    for idx,pitch in enumerate([root-12,root-12+third,root-5,root+2]):
        note(base,3.8,pitch,'strings',.022 if section!=3 else .03,(-.65,-.25,.25,.65)[idx])
    for step,pitch in enumerate([root,root+7,root+third,root+7]):
        if section==0 and step%2: continue
        note(base+step+.04, .65, pitch,'piano',.047, -.35)
    for offset,pitch,length in [(0,root-24,1.6),(2,root-17,1.5)]:
        note(base+offset,length,pitch,'bass',.085)
    if section!=0 or bar>=4:
        phrase = bridge[bar%4] if section==3 else motifs[(bar + (2 if section in (2,5) else 0))%8]
        transpose = -12 if section==3 else (5 if section==2 else 0)
        # Distinct B/outro melody contours and alternate flute/vibes timbre.
        for offset,length,pitch in phrase:
            note(base+offset,length,pitch+transpose,'vibes' if section in (2,5) else 'flute',.064,.25)
    if section==4 and bar%2==1:
        for step in (1,2.5): note(base+step,.65,root+12+third,'piano',.025,-.55)
    if section not in (0,3):
        for beat in (0,2): drum(base+beat,'kick',.10)
        for beat in (1,3): drum(base+beat,'snare',.11)
        for beat in (0,.5,1,1.5,2,2.5,3,3.5): drum(base+beat,'hat',.025 if beat%1 else .018)
        if bar%8==7:
            for beat in (3.25,3.5,3.75): drum(base+beat,'snare',.035)

# A gentle stereo echo/reverb return, inspired by console sample playback.
dry=mix.copy()
for seconds,gain in [(.113,.11),(.193,.14),(.277,.10),(.389,.065),(.521,.04)]:
    n=int(seconds*RATE)
    mix[n:,0] += dry[:-n,1]*gain
    mix[n:,1] += dry[:-n,0]*gain
# Roll off harsh treble. Equal-energy stereo and conservative dynamics.
for channel in range(2):
    mix[:,channel]=np.convolve(mix[:,channel],np.array([1,2,3,2,1])/9,'same')
mix-=mix.mean(axis=0)
peak=float(np.abs(mix).max());mix*=.24/peak
fade=int(.7*RATE);mix[:fade]*=np.linspace(0,1,fade)[:,None];mix[-fade:]*=np.linspace(1,0,fade)[:,None]
parser=argparse.ArgumentParser();parser.add_argument('output',type=Path);args=parser.parse_args()
args.output.parent.mkdir(parents=True,exist_ok=True)
with wave.open(str(args.output),'wb') as f:
    f.setnchannels(2);f.setsampwidth(2);f.setframerate(RATE)
    f.writeframes((mix*32767).astype('<i2').tobytes())
print(f'{args.output}: {DURATION:.2f}s, 80 bars, {BPM} BPM, stereo 32kHz PCM; RMS {np.sqrt(np.mean(mix*mix)):.4f}, peak {np.abs(mix).max():.4f}')
