This directory contains a 1-Q QR code containing the string HELLO WORLD as text.
Used for a proof of concept program to determine whether QR codes can be read
from the Beeb's monitor.

`hello-world-final.png`: Original file.

`hello-world-final.pgm`: Processed in GIMP: removed 1px black border, removed
quiet zone, scaled down so 1 pixel equals 1 module, and saved as PGM.

`process.cc`: Converts the PGM from stdin to a BASIC program to render the QR
code on stdout.

Source: <http://www.thonky.com/qr-code-tutorial/format-version-information/>
