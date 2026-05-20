from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
import pandas as pa
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
from pykstars import ObjectType
from dataclasses import dataclass


@dataclass
class BarnardRecord:
    number: int
    name: str
    long_name: str
    ra: float
    dec: float
    major_axis: float
    ra_h: float
    ra_m: float
    ra_s: float
    dec_sign: str
    dec_h: float
    dec_m: float
    diam_str: str  # Items after this are filled in as the data is read in
    ra: float
    dec: float
    major_axis: float
    trixel: int
    type: int
    magnitude: int  # Made up value... it is not included in the catalog


class Barnard(Factory):
    meta = Catalog(
        id=7,
        name="Barnard Planetary Nebulae",
        author="George O. Barnard",
        description="""The Barnard Catalog of Planetary Nebulae was created in 1966 by George O. Barnard and was composed of 86 entries thought to be planetary nebulae that were collected from discoveries, about half by Albert George Wilson and the rest by Barnard, Robert George Harrington, and Rudolph Minkowski. All were discovered before August 1955 as part of the National Geographic Society – Palomar Observatory Sky Survey on photographic plates created with the 48-inch (1.2 m) Samuel Oschin telescope at Mount Palomar.""",
        source="<a href='https://astronomy-mall.com/Adventures.In.Deep.Space/abellcat.htm'>Adventures in Deep Space</a>",
        precedence=0.3,
        version=1,
        maintainer="Carl Knight <SleeplessAtKnight@gmail.com>",
        license="Free for non-commercial use only",
        colors={"default": "#dbffd5"},
        image="barnard.jpg",
    )
    records = []

    def load_objects(self):

        with open(self._in_data_dir("barnard.dat"), "rt") as ifp:
            print("Opened barnard.dat")
            lines = ifp.read().split("\n")
            print(f"Barnard has {len(lines)} records")
            # catalog = pa.read_html(self._in_data_dir("abellcat.htm"), header=0)[0]
            catalog = "Barnard"
            self._state = catalog
            for l in lines:
                obj = self.parse_record(l)
                self.records.append(
                    obj
                )  # We need to cache as we will use this in the De-Dupe phase also
                yield self._make_catalog_object(
                    type=ObjectType.DARK_NEBULA,
                    ra=obj.ra,
                    dec=obj.dec,
                    magnitude=obj.magnitude,
                    name=obj.name,
                    catalog_identifier=obj.name,
                    major_axis=obj.major_axis,
                    minor_axis=0.0,  # Not mentioned in Catalog source
                )

    def get_dublicates(self, query_fn, catalogs):
        if (
            open_ngc.OpenNGC.meta.id not in catalogs
            and ngcic_steinicke.NGCICSteinicke.meta.id not in catalogs
        ):
            return []

        for obj in self.records:
            name: obj.name

            if name.startswith("NGC") or name.startswith("IC"):
                this_object = query_fn(
                    self.meta.id, f"catalog_identifier = {obj['BARN']}"
                )

                if len(this_object) < 1:
                    continue

                this_object = this_object[0]

                suspects = query_fn(
                    CATALOGS.all_objects,
                    f"trixel = {this_object.trixel} AND name LIKE '{name}'",
                )

                if len(suspects) > 0:
                    yield {(self.meta.id, this_object.hash)}.union(
                        {(suspect.catalog, suspect.hash) for suspect in suspects}
                    )

    def parse_record(self, line: str) -> BarnardRecord:
        """
        This is the record definition as specified in the source file

          2-  5  A4     ---    Barn     *[ 0-9a]! Barnard number
          6-  7  I2     h      RAh       Right Ascension 1875 (hours)
          9- 10  I2     min    RAm       Right Ascension 1875 (minutes)
         12- 13  I2     s      RAs      *? Right Ascension 1875 (seconds)
             15  A1     ---    DE-       Declination 1875 (sign)
         16- 17  I2     deg    DEd       Declination 1875 (degrees)
         19- 20  I2     arcmin DEm       Declination 1875 (minutes)
         23- 24  I2     h      RA2000h   Right Ascension 2000 (hours)
         26- 27  I2     min    RA2000m   Right Ascension 2000 (minutes)
         29- 30  I2     s      RA2000s   ? Right Ascension 2000 (seconds)
             33  A1     ---    DE2000-   Declination 2000 (sign)
         34- 35  I2     deg    DE2000d   Declination 2000 (degrees)
         37- 38  I2     arcmin DE2000m   Declination 2000 (minutes)
         40- 44  F5.1   arcmin Diam      ? Diameter of the nebula
        """
        s = BarnardRecord(
            number=1,  # Updated output the parse due to 44 44a 44b etc
            name="BRN " + line[2:5].strip(),
            ra_h=float("0" + line[6:7]),
            ra_m=float("0" + line[9:10]),
            ra_s=float("0" + line[12:10]),
            dec_sign=line[32:33],
            dec_h=float("0" + line[34:35]),
            dec_m=float("0" + line[37:38]),
            diam_str=line[40:],
            ra=0,
            dec=0,
            major_axis=0,
            long_name="",
            trixel=0,
            magnitude=8,
            type=ObjectType.DARK_NEBULA,
        )

        # Calculate the RA / DEC

        s.ra = s.ra_h + s.ra_m / 60.0 + s.ra_s / 3600.0
        sign = 0
        if s.dec_sign == "-":
            sign = -1
        else:
            sign = 1
        s.dec = (s.dec_h + s.dec_m / 60.0) * sign
        tmp_d = s.diam_str.strip().strip("\n")
        if len(tmp_d) == 0:
            tmp_d = "0.0"
        s.major_axis = float(tmp_d)
        s.long_name = s.name
        print(s)
        return s
