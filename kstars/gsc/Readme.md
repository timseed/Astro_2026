# GSC Guide Star Catalog 

Kstars uses this "under the hood" when using the CCD/Guider Simulators. 

It was somewhat difficult to track down - hence it is is included here. 

# Build 

Usual cmake - easy.

# install 

My gsc is installed as  */usr/local/bin/*

with the following Env variables 


    INDI_GSC_DIR=/usr/local/share/GSC/bin/
    GSCDAT=/usr/local/share/GSC
    GSCBIN=/usr/local/share/GSC/bin


## Files 

The **GSCDAT** folder looks like this 


```
bin    N0730  N2230  N3730  N5230  N6730  N8230  S0730  S2230  S3730  S5230  S6730  S8230
N0000  N1500  N3000  N4500  N6000  N7500  S0000  S1500  S3000  S4500  S6000  S7500
```


The **GSCBIN** folder looks like this 

```text
regions.bin  regions.ind
```


