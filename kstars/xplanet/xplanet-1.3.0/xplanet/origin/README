The format of an origin file is
YYYYMMDD.HHMMSS	 range	lat  lon  [localtime]

Where the date is in GMT and range is in planetary radii.  The values
for range, lat, and lon refer to the body named with the -origin
option.  The localtime is optional, but if supplied, it will be used
in place of the longitude.

There are two origin files included with xplanet.  One describes the
path of the Galileo spacecraft on its approach to Jupiter, and the
other is for Cassini as it arrives at Saturn.  An example use is

xplanet -origin jupiter -target io -origin_file galileo -wait 10

This updates the image every 10 seconds using the next date in the
origin file.
