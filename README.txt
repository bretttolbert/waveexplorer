sudo apt-get install libqt4-dev
qmake
make

maxSampleVal=5319

waveformWidgetHeight=400 so range should be -200 to 200

my "num samples" calculation is wrong because it went out of bounds here:
numSamples=366592 x=183810

guitar track is 8.25 seconds long

8.25 * 44100 = 363825 samples per track
8.25 * 2 * 44100 = 727650 samples total
727650 * 2 = 145530 bytes or 1421 KB + 36 = filesize on disk 1433 KB

