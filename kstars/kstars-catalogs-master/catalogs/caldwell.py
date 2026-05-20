from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
import pandas as pa
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
from pykstars import ObjectType
import re


def parse_hms(s):
    # print(f"HMS  Parse {s}")

    parts = re.findall(r"([\d]+)[ ]([\d]+[\.][\d]+)", s)
    h, m_sec = map(float, parts[0])
    return h + m_sec / 60  # decimal hours


def parse_dec(s):
    # print(f"DEC  Parse {s}")
    parts = re.findall(r"([+-])([\d]+)[ ]([\d]+)", s)[0]
    sign = -1 if parts[0].startswith("-") else 1
    deg = float("0" + parts[1])
    minu = float("0" + parts[2])
    return sign * (deg + minu / 60)


def parse_size(s):
    """
    It appears we have 3 types of size
    14,31,0.6 etc
    21x7,18x18
    40_35

    we will return a tuple of Major, Minor size
    """
    rv = []

    parts = re.findall(r"(\d+\.?\d*)[\D]+(\d+\.?\d*)", s)
    # We are expecting 1 record, with 2 fields
    if len(parts) == 1:
        rv.append(parts[0][0])
        rv.append(parts[0][1])
        # print(f"{rv}")
    else:
        rv.append(s)
        rv.append(s)
        # print(f"{rv}")
    return rv


class Caldwell(Factory):
    meta = Catalog(
        id=10,
        name="Caldwell",
        author="Sir Patrick Moore",
        description="""In 1995, Sir Patrick Caldwell-Moore noted that the Messiar catalog does not include a number of bright deep sky objects, nor does it cover any Southern Hemisphere objects south of declination -35""",
        source="<a href='https://astropixels.com/caldwell/caldwellcat.html'</a>",
        precedence=0.3,
        version=1,
        maintainer="Tim Seed<tim@sy-edm.com>",
        license="Free for non-commercial use only",
        colors={
            "default": "#4DA6FF",
            "night.colors": "#ff6164",
            "moonless-night.colors": "#ff6164",
        },
        image="caldwell.jpg",
    )
    records = []

    def load_objects(self):

        df = pa.read_csv(self._in_data_dir("caldwell.txt"), sep="\t")

        # df.info()

        # make sure all fields are populated
        df["Common Name"] = df["Common Name"].fillna(df["NGC"])
        df["RA_2"] = df["RA (hr_min)"].apply(parse_hms) / 24.0 * 360.0
        df["DEC_2"] = df["Dec (dg_min)"].apply(parse_dec)
        # 4 Items have no magnitude
        # East, West Veil. Rosette and Coalsack Nebula
        # I have set them as 8
        df["Mag_str"] = df["Mag."].replace("-", "8")
        df["Mag_2"] = df["Mag_str"].astype(float)
        df.rename(columns={"Common Name": "Alt_name"})

        # This is from the C++ Code
        # KStars 3.8
        KSTARS_TYPE_MAP = {
            # Stars
            "STAR": 1,
            "S": 1,
            "TYCHO": 1,
            # Solar system
            "PLANET": 2,
            "ASTEROID": 10,
            "COMET": 9,
            "SATELLITE": 16,
            # Clusters
            "OPEN": 3,
            "OPEN_CLUSTER": 3,
            "OC": 3,
            "GLOBULAR": 4,
            "GC": 4,
            "GLOBULAR_CLUSTER": 4,
            # Nebulae
            "GASEOUS": 5,
            "HII": 5,
            "DIFFUSE": 5,
            "EMISSION": 5,
            "PLANETARY": 6,
            "PN": 6,
            "SUPERNOVA_REMNANT": 7,
            "SNR": 7,
            # Galaxies
            "GALAXY": 8,
            "G": 8,
            "CLUSTER": 14,  # galaxy clusters
            "QUASAR": 15,
        }

        mapping = {
            "Oc": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Gc": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Sp": KSTARS_TYPE_MAP["GALAXY"],  # Spiral Galaxy
            "Pl": KSTARS_TYPE_MAP["PLANETARY"],
            "Bn": KSTARS_TYPE_MAP["EMISSION"],
            "Sb": KSTARS_TYPE_MAP["GALAXY"],  # Inclined Spiral Galaxy ?
            "El": KSTARS_TYPE_MAP["CLUSTER"],  # Super Eliptical Galaxy
            "Ir": KSTARS_TYPE_MAP["GALAXY"],  # irregular dwarf galaxy
            "Sn": KSTARS_TYPE_MAP["SUPERNOVA_REMNANT"],
            "Sx": KSTARS_TYPE_MAP["QUASAR"],  # Seyfert galaxy - Quasar
            "Px": KSTARS_TYPE_MAP["GALAXY"],  # Lenticular galaxy
            "Dn": KSTARS_TYPE_MAP["GASEOUS"],  # Dark Nebulae
        }

        print("Finished loading the Data")
        df["KStars_Type"] = df["Type"].map(mapping)

        # df['Type'].value_counts()

        df["tmp_name"] = df["Common Name"].fillna(df["NGC"])

        tmp_size_series = df["Size arc_min"].apply(lambda x: parse_size(x))
        df[["major_axis", "minor_axis"]] = pa.DataFrame(
            tmp_size_series.tolist(), index=df.index
        )

        self._state = df
        # for _, obj in self._state.iterrows():
        for i, (_, obj) in enumerate(self._state.iterrows()):
            yield self._make_catalog_object(
                type=int(obj["KStars_Type"]),
                ra=float(obj["RA_2"]),
                dec=float(obj["DEC_2"]),
                magnitude=float(obj["Mag_2"]),
                name=str(obj["tmp_name"]),
                long_name=str(obj["tmp_name"]),
                catalog_identifier=str(f"COL {i + 1}"),
                major_axis=float(obj["major_axis"]),
                minor_axis=float(obj["minor_axis"]),  # Not mentioned in Catalog source
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
                    self.meta.id, f"catalog_identifier = {obj['tmp_name']}"
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
