# spec ("Adobe Camera Model")
# https://download.macromedia.com/pub/labs/lensprofile_creator/lensprofile_creator_cameramodel.pdf

# lens profiles
# https://7artisans.store/pages/calibration-profiles
# https://www.ttartisan.com/?LensProfile/
# /Library/Application\ Support/Adobe/CameraRaw/LensProfiles/

import matplotlib.pyplot as plt
import numpy as np
from collections import namedtuple


long_side = 36.0
short_side = 24.0
diag = np.sqrt(long_side ** 2 + short_side ** 2)

w = np.linspace(0, 220 / 2 * np.pi / 180, 100)

f = 14
r_equidistance = f * w
r_equisolid = f * 2 * np.sin(w / 2)
f = 11
r_stereograhic = f * 2 * np.tan(w / 2)
f = 10
r_orthographic = f * np.sin(w)
w_rectlinear = np.linspace(0, 140 / 2 * np.pi / 180, 100)
r_rectlinear = f * np.tan(w_rectlinear)

plt.figure(figsize=(8, 6))

plt.axhline(diag/2, label='diag', color='gray', linewidth=2)
plt.axhline(long_side/2, label='long side', color='gray', linewidth=2)
plt.axhline(short_side/2, label='short side', color='gray', linewidth=2)
plt.axvline(np.pi / 2, label='pi / 2', color='gray', linewidth=2)

plt.plot(w_rectlinear, r_rectlinear, label='rectlinear 10mm', linewidth=2)
plt.plot(w, r_equidistance, label='equidistance 14mm', linewidth=2)
plt.plot(w, r_equisolid, label='equisolid 14mm', linewidth=2)
plt.plot(w, r_orthographic, label='orthographic 10mm', linewidth=2)  # eg. OP fisheye nikkor 10mm F5.6
plt.plot(w, r_stereograhic, label='stereographic 11mm', linewidth=2)

Lens = namedtuple(
	"LensProfile",
    [
        "f",  # FocalLengthX/FocalLengthY (if absent, FocalLength / long_side)
        "k1",  # RadialDistortParam1
        "k2",  # RadialDistortParam2
    ]
)

lenses = {
    "7artisans 6mm F2": Lens(0.343383, -0.001042, -0.004391),
    "7Artisans 7.5mm 2.8 II": Lens(0.365239, 0.008585, 0),
    "7artisans 10mm F2.5": Lens(0.326861, 0.020230, 0.012446),
    "TTARTISAN MF 7.5mm F2 Fisheye": Lens(0.437334, -0.051090, 0),
    "TTARTISAN MF 11mm F2.8 Fisheye": Lens(0.448150, -0.024293, -0.010949),
}

for k, v in lenses.items():
    f = v.f * long_side
    k1 = v.k1
    k2 = v.k2
    # lensprofile_creator_cameramodel.pdf
    # p5. "Geometric Distortion Model for Fisheye Lenses"
    r = f * (w + k1 * np.pow(w, 3) + k2 * np.pow(w, 5))
    plt.plot(w, r, label=k, linewidth=2)

plt.title('fisheye lens projection', fontsize=14)
plt.xlabel('angle', fontsize=12)
plt.ylabel('radius', fontsize=12)

plt.legend()
plt.grid(True)
plt.show()
