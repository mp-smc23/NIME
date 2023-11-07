from matplotlib import pyplot as plt

def note_from_index(i):
    n = ["C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"]
    return n[i % 12] + str(int(i / 12))

def pitch_from_index(i):
    # C0 is 0
    return 2**((i-57)/12)*440

def note_from_distance(i):
    sw = 20 # step width in mm
    x_offset = 100
    x_interval = 900
    first_note = 40 # E3
    last_note = 60 # C5
    ss = x_interval / (last_note - first_note)
    if i < x_offset:
        return first_note
    elif i >= x_offset + x_interval:
        return last_note
    else:
        d = i - x_offset
        if d % ss < sw / 2:
            return first_note + int(d / ss)
        elif d % ss >= ss - sw / 2:
            return first_note + int(d / ss) + 1
        else:
            return first_note + int(d / ss) + (d % ss - sw / 2) * 1 / (ss - sw)


def loudness_comp(p):
    # p is pitch in hertz
    x = [   0,      20,	    25,	    31.5,	40,	    50,	    63,	    80,	    100,	125, 	160,    200,	250,	315, 	400, 	500, 	630, 	800, 	1000,	1250,	1600,	2000,	2500,	3150,	4000,	5000,	6300,	8000,	10000,	12500,	16000,	20000]
    y = [   145.0,  118.99,	114.23,	109.65,	105.34,	101.72,	98.36,	95.17,	92.48,	90.09,	87.82,	85.92,	84.31,	82.89,	81.68,	80.86,	80.17,	79.67,	80.01,	82.48,	83.74,	80.59,	77.88,	77.07,	78.31,	81.62,	86.81,	91.41,	91.74,	85.41,	84.67,	118.95]
    for i in range(len(x) - 1):
        if p >= x[i] and p < x[i+1]:
            dy = y[i+1] - y[i]
            dx = x[i+1] - x[i]
            return dy/dx * (p - x[i]) + y[i] 
    return 0

# https://github.com/andrewjhunt/equal-loudness


fig, axs = plt.subplots(3)
fig.suptitle('duh')


notes = []
pitches = []
for i in range (30, 70):
    notes.append(note_from_index(i))
    pitches.append(pitch_from_index(i))
axs[0].plot(notes, pitches)
axs[0].set_title("note vs. pitch [Hz]")


pitches = []
gains = []
for i in range (0, 100):
    pitch = pitch_from_index(i)
    pitches.append(pitch)
    gains.append(loudness_comp(pitch))
axs[1].plot(pitches, gains)
axs[1].set_xscale('log')
axs[1].set_title("pitches [Hz] vs. gains [dB]")


distances = []
mapped_pitches = []
for i in range (0, 1100):
    distances.append(i)
    mapped_pitches.append(pitch_from_index(note_from_distance(i)))
axs[2].plot(distances, mapped_pitches)
axs[2].set_title("distances [mm] vs. pitches [Hz]")


plt.show()