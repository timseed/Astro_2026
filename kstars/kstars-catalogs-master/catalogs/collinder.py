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
    parts = re.findall(r"([\d]+)[\D]{2}([\d]+)[\D]{2}([\d]+[\.][\d]+)", s)
    if len(parts) == 1:
        deg = float(parts[0][0])
        minu = float(parts[0][1])
        sec = float(parts[0][2])
        return deg + minu / 60.0 + sec / 3600.0
    else:
        return 0.0


def parse_dec(s):
    parts = re.findall(r"([+-])([\d]+)[\D]{2}([\d]+)[\D]{2}([\d]+)", s)
    if len(parts) == 1:
        if parts[0][0] == "+":
            sign = 1
        else:
            sign = -1
        deg = float(parts[0][1])
        minu = float(parts[0][2])
        sec = float(parts[0][3])

    return sign * (deg + minu / 60.0 + sec / 3600.0)


class Collinder(Factory):
    meta = Catalog(
        id=9,
        name="Collinder",
        author="Per Collinder",
        description="""The Collinder catalogue is a catalogue of 471 open clusters compiled by Swedish astronomer Per Collinder. It was published in 1931 as an appendix to Collinder's paper On structural properties of open galactic clusters and their spatial distribution.""",
        source="<a href='https://en.wikipedia.org/wiki/Collinder_catalogue'</a>",
        precedence=0.4,
        version=1,
        maintainer="Tim Seed<tim@sy-edm.com>",
        license="Free for non-commercial use only",
        colors={"default": "#dbffd5"},
        image="collinder.jpg",
    )
    records = []

    def load_objects(self):

        df = pa.read_csv(self._in_data_dir("col.txt"), sep="\t")

        df.rename(
            columns={
                "Col #": "Id",
                "NGC/Other Cat.": "NGC_Other",
                "m ( v/p)": "mag_v_p",
                "# Stars": "SC",
                "n": "Note",
            },
            inplace=True,
        )
        # Convert to Format of the RA and Dec
        df["RA_degrees"] = df["RA"].apply(parse_hms) / 24.0 * 360.0
        df["DEC_degrees"] = df["DEC"].apply(parse_dec)
        df["NGC_Other"] = df["NGC_Other"].fillna(
            ""
        )  # There were a few which are not NGC

        df["mag_v_p"] = df["mag_v_p"].fillna(10)  # There is only 1 Id 240
        df["Size"] = df["Size"].fillna(10)  # There is only 1 Id 240
        df["Note"] = df["Note"].fillna(0)  # 403 without a Note
        # So now we have no Na Values

        # extract text inside parentheses
        df["Alt_name"] = df["NGC_Other"].str.extract(r"\((.*?)\)")
        df["Alt_name"] = df["Alt_name"].fillna("")  # 35 have alternative Names
        # remove parentheses part
        df["NGC_IC"] = df["NGC_Other"].str.replace(r"\s*\(.*?\)", "", regex=True)
        df["Star_Count"] = df["SC"].str.extract(r"\(?(\d+)\)?")
        df["Star_Count"] = df["Star_Count"].fillna(0)

        df["Mag"] = df["mag_v_p"].str.extract(r"\(?(\d+)\)?")
        df["Mag"] = df["Mag"].fillna(8)  # 8 without magnitude

        df["Mag_Type"] = df["mag_v_p"].str.extract(r"([vp])")
        df["Mag_Type"] = df["Mag_Type"].fillna("v")  # Just 1
        df.drop(columns=["SC", "mag_v_p"], inplace=True)

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

        # Note: Mapping was obtained by
        #    df['Class'].value_counts()
        # This is not DYNAMIC

        mapping = {
            "Plei": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Praes": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Neb": KSTARS_TYPE_MAP["EMISSION"],
            "μNorm": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Glob": KSTARS_TYPE_MAP["GLOBULAR"],
            "nl": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Chain": KSTARS_TYPE_MAP["OPEN_CLUSTER"],
            "Neb?": KSTARS_TYPE_MAP["EMISSION"],
            "(Neb)": KSTARS_TYPE_MAP["EMISSION"],
        }
        print("Finished loading the Data")
        df["KStars_Type"] = df["Class"].map(mapping)
        self._state = df
        # for _, obj in self._state.iterrows():
        for i, (_, obj) in enumerate(self._state.iterrows()):
            yield self._make_catalog_object(
                type=int(obj["KStars_Type"]),
                ra=float(obj["RA_degrees"]),
                dec=float(obj["DEC_degrees"]),
                magnitude=float(obj["Mag"]),
                name=str(f"COL {1 + i}"),
                long_name=str("NGC" + obj["NGC_Other"]),
                catalog_identifier=str(f"COL {i}"),
                # major_axis=float(obj.major_axis,
                #    minor_axis=0.0,  # Not mentioned in Catalog source
            )

    def get_dublicates(self, query_fn, catalogs):
        if (
            open_ngc.OpenNGC.meta.id not in catalogs
            and ngcic_steinicke.NGCICSteinicke.meta.id not in catalogs
        ):
            return []
        i = 0
        for obj in self.records:
            name: obj.name
            i = i + 1
            tmp_id = str(f"COL {i}")
            if name.startswith("NGC") or name.startswith("IC"):
                this_object = query_fn(self.meta.id, f"catalog_identifier = {tmp_id}")

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

    def ks_catagory(self, df) -> list:
        """
                Create a Series for KStar Catagories
                The GaryImm data set has Type and Sub-Type

                Gal      1461
                Neb      1324
                Neb         3
                Stars     357

                But when I add Sub-Type this is a little more Complex

        Gal    BCD          14
               Chain        16
               Cluster      47
               Coll         21
               Dwarf        19
               Ellip        63
               Floc         24
               Group       144
               Group         2
               Lent        148
               Mag          57
               Merger      175
               Pair         74
               Pair          1
               Polar        11
               Spiral      624
               Spiral        2
               Trio         19
        Neb    Dark        372
               Em          366
               Mol CLd       1
               Mol Cld      47
               PN          308
               PPN           8
               Refl        191
               SNR          31
        Neb    Em            3
        Stars  GC           96
               HH           12
               Nova          3
               OC          214
               Star          2
               Star Cld      2
               Stars         2
               YSO          26


                :param df:
                :return:
        """

        result = []
        for row in df[["Type", "sub_type"]].itertuples(index=True, name="src"):
            rv = 1  # A Star in kstars terms
            match row.Type:
                case "Stars":
                    match row.sub_type:
                        case "GC":
                            rv = ObjectType.GLOBULAR_CLUSTER.value
                        case "Nova":
                            rv = ObjectType.SUPERNOVA.value
                        case "OC":
                            rv = ObjectType.OPEN_CLUSTER.value
                        case _:
                            rv = ObjectType.STAR.value
                case "Neb":
                    match row.sub_type:
                        case "Em":
                            rv = ObjectType.GASEOUS_NEBULA.value
                        case "Dark":
                            rv = ObjectType.DARK_NEBULA.value
                        case "PN":
                            rv = ObjectType.PLANETARY_NEBULA.value
                        case "PNN":
                            rv = ObjectType.PLANETARY_NEBULA.value
                        case _:
                            rv = ObjectType.GASEOUS_NEBULA.value
                case "Gal":
                    match row.sub_type:
                        case "Group":
                            rv = ObjectType.GALAXY_CLUSTER.value
                        case "Pair":
                            rv = ObjectType.GALAXY_CLUSTER.value
                        case _:
                            rv = ObjectType.GALAXY.value
                case _:
                    print(f"Unknown Type <{row.Type}>")
                    rv = ObjectType.TYPE_UNKNOWN.value
            result.append(rv)
        return result
