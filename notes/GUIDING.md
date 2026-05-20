
Run With 

indiserver indi_ioptronv3_telescope indi_svbony_ccd indi_toupcam_focuser

## PRIMARY 

Mount should be the iOptron
Camera is SV405 

## Guiding

Use phd2 

Setting | Value 
RA/Dec Agr|  75
Hys | 25 
MinMo | 0.15 (Both Axis)
MX | 2000 (Both) 


| --------------- | --------------- | --------------- | --------------- | --------------- |
| Item1.1 | Item2.1 | Item3.1 | Item4.1 | Item5.1 |

I now update every 4 seconds. This seems to smooth things out very nicely. But balance is the #1. 

# Ekos Guiding 

I finally got it working !!
This is the process... 
Use a Moon/Dedicated Video control software which allows you to see full image what the guide camera is doing. 

For me I needed the Contrast turned way high, gain made little or no difference. I also tweaked the White Balance. 

Then using EKOS ... I chose the **Auto Threshold** algorithm. I set the Agressivness similar to what I would use in PHD2, and then did the following 

  - Capture Image 
  - Click on 1 Star and set the Box width so it is tight around the *white* of the star. 
    - Now press SubImage 
  - Press Capture 
    - Check your selected star is visible ....
  - Press Guide 

# SV165 Guide Scope 

Focal Length 160mm 
Aperture width 40mm 
NOT the 120 I previously had thought of !!!

# 905C settings 

16 Bit Raw 
Gain 50 
Gamma 60
Contrast 80

WB R = 284
WB B = 284
WB G = 394 
This gives a blue background


# KStars Guiding 

I have a program called **gsp** 

Take an image with the guide scope, and then press the *galaxy* button (this will open it in Fits Viewer)

  - Next Click on the Histogram (Verticle with lines) and look at the statistics. 
    - We need to have a Calculated HFR 
    - If we do not keep adjusting the exposure/focus etc until we have an HFR 

Now with an HFR say (0.973) 

  - Guiding
    - Options 
      - Go and set the min pixel value to being the same (round up as the calculated HFR)
      - Try Guiding now
