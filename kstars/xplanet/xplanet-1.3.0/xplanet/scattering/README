Rayleigh scattering is implented using lookup tables.  Tables are
calculated for fixed phase angles (the observer - point - sun angle).
For lines of sight intersecting the disk, the tables are a 2D array of
scattering intensities tabulated as a function of incidence (the
zenith angle of the sun seen from the point of interest) and emission
(the zenith angle of the observer seen from the point of interest)
angles. 

The lookup tables can be calculated using the sample scattering file
in the xplanet distribution:

xplanet -create_scattering_tables earthRayleigh -verbosity 1

The comments in the scattering file describe its format.  Once the
tables have been created, place them in the xplanet/scattering
directory.
