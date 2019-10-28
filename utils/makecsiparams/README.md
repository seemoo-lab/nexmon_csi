Tool for generating base64 encoded strings that can be used for configurating the [Nexmon Channel State Information Extractor](https://nexmon.org/csi).
You can build it by running `make`. Parameters are:  
```
Usage: makecsiparams [OPTION...]

   -h           print this message
   -e on/off    enable/disable CSI collection (0 = disable, default is 1)
   -c chanspec  Channel specification <channel>/<bandwidth>
   -C coremask  bitmask with cores where to activate capture
                (e.g., 0x5 = 0b0101 set core 0 and 2)
   -N nssmask   bitmask with spatial streams to capture
                (e.g., 0x7 = 0b0111 capture first 3 ss)
   -m addr      filter on this source mac address (up to four, comma separated)
   -b byte      filter frames starting with byte
   -d delay     delay in us after each CSI operation
                (really needed for 3x4, 4x3 and 4x4 configurations,
                without it is enforced automatically)
   -r           generate raw output (no base64)
```
