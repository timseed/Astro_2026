# 

This is my attempt to "grab" IBLOB files - which are sent when an image is captured.

## Indi 

indi_getprop will show which devices are available.

# Flow Control 

We first Start the EKOS/Indi-Server in this case I am using the simulator.

    ./start_simserver.sh 

This will **tail** a log file called ~/simsvr_log.log so I can see what is going on. 

## Next start my "grabber" 

Again this is a simple process 

    ./indiblob 

I can see it connecting, this is how to verify it is connected 


    lsof -i tcp:7624 

The output I see is 

```text
indiserve 1361  tim  5u  IPv4  14728      0t0  TCP *:indi (LISTEN)
indiserve 1361  tim  8u  IPv4  17787      0t0  TCP localhost:indi->localhost:55428 (ESTABLISHED)
indiserve 1361  tim 17u  IPv4  14945      0t0  TCP astrodev:indi->m1:62618 (ESTABLISHED)
indiserve 1361  tim 18u  IPv4  14946      0t0  TCP astrodev:indi->m1:62619 (ESTABLISHED)
indiserve 1361  tim 19u  IPv4  14947      0t0  TCP astrodev:indi->m1:62620 (ESTABLISHED)
indiserve 1361  tim 20u  IPv4  14948      0t0  TCP astrodev:indi->m1:62621 (ESTABLISHED)
indiblob  2656  tim  5u  IPv4  16091      0t0  TCP localhost:55428->localhost:indi (ESTABLISHED)
```
 
