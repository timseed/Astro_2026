# Guiding

## Guide Scope/Pixel Ratio 

Guide Camera 

  - The SV905C pixel size ≈ 3.75 µm

Guidescope focal length:

  - 120mm @ f/4 → 30mm Apature 

```  
scale=206×pixel size
          ----------
         focal length
```	

```
206*3.75
   ------
   120.0 
```
   
≈6, So each pixel is 6 arc seconds. 

## Ekos Setting 

Since each pixel is 6.4″, the guider must ignore tiny movements. 

  - Exposure Length
     - 2-3 Seconds 
  - Min Move 
     - 0.20 – 0.30 px 
     - which is 0.25 px × 6.4 ≈ 1.6 arcsec
  - Aggression 
     - RA 
	   - 0.6
	 - DEC
       - 0.8 
   - RA max pulse 
      - 1000 ms
   - DEC max pulse 
      - 1500 ms	   
   - Calibration pulse 
      - 1500 ms 
	 
## Expected Guiding 

  - RA RMS 
    - 0.7–1.2"
  - DEC RMS 
    - 0.8–1.4"
  - Total RMS 
    - 1.0–1.6"
  
  
  