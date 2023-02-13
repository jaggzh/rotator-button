# Rotation-based Button - Self-calibrating

This is the Arduino sketch/project for a self-calibrating self-zeroing button for use by a patient.

[![Video of 3d model of hand-held (palm-held) button:](https://img.youtube.com/vi/lR_kUQ13Uhk/maxresdefault.jpg)](https://www.youtube.com/watch?v=lR_kUQ13Uhk)

## Questions

I can be contacted at jaggz.h {who can be found at} gmail.com, or in irc.libera.chat as jaggz (usually in ##electronics or #reprap).

## Purpose and Background

A patient with limited range of motion can have a difficult if not impossible time using a normal button.  They may get their finger(s) stuck down, holding it triggered, or be unable to press it at all.  With many hours and months of time wasted with constant fine adjustments of the patient and button's position, I decided to find a solution.  This was used by a patient to trigger a feeding-tube pump, so its safe comfortable operation was essential.

Originally I designed a capacitive sensing hand-held button.  It self-calibrated based on the amount of capacitance of the patient's rest position -- a squeeze resulted in a higher capacitance.  I almost got it working but there were too many complications, which I won't go into here, and it became low priority for a while.  When returning to it, I decided to, instead, use an accelerometer/gyro board, and it's been working flawlessly.

## Technical details

### Sensor data handling:

This currently uses an MPU9250 board, but a prior version used an MPU6050.  I'm only using a single axis of accelerometer data, so there are tons of options out there for it.

With sensor fusion (merging accelerometer, gyroscope, and possibly magnetometer data), an absolute angle of the button is possible.  In practice, there was a lot of drift, and complications with the board orientation due to the libraries involved.  As such, I changed to just using one axis's accelerometer value -- its angle away from gravity.  To remove sudden spikes in acceleration value (from movement or button mechanics), I use a median-filter on the data.  I do other filtration, like slow-moving averages, which are biased to increase the range quicker in one direction than the other, etc.

This is an older display of some of the data.  The newer processing and output are more elaborate but I don't have a video of it yet:

[![Video of accelerometer data, median filtered result, and max/min envelopes](https://img.youtube.com/vi/tvhn7LEv6tM/maxresdefault.jpg)](https://www.youtube.com/watch?v=tvhn7LEv6tM)


### 3d design

1. I designed the button in Blender 3d.
1. To do this, I took a mold of the patient's grip by having her grab a lump of plaster (plaster of paris with too little water in it).
	1. Then I marked this up with markers for 3d photogrammetry (it wasn't Meshroom, but I'd probably use Meshroom for this project were I to do it today).  This provided a 3d mesh to which I could model a final "clean" button set.
1. This version is really just 2 parts (although I have a separate button for reset/other things on the side. This can be ignored), soooo... Technically it's just printing the bdoy and the button.  The button has slices in it which let it be squeezed so its hinges can just pop right into place.

