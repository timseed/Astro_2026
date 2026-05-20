from lib.catalogfactory import Factory, Catalog
from lib.catalogsdb import CATALOGS
import pandas as pa
import os.path
import pdb
from astropy import units as u
from astropy.coordinates import Angle
from . import open_ngc, ngcic_steinicke
from pykstars import ObjectType


class GaryImm(Factory):
    meta = Catalog(
        id=8,
        name="Gary Imm",
        author="Gary Imm",
        description="""Gary Imms very impressive list of interesting objects and features.""",
        source="<a href='https://GaryImm/abellcat.htm'>Adventures in Deep Space</a>",
        precedence=0.4,
        version=1,
        maintainer="Tim Seed<SleeplessAtKnight@gmail.com>",
        license="Free for non-commercial use only",
        colors={"default": "#dbffd5"},
        image="garyimm.jpg",
    )
    records = []

    def load_objects(self):

        df = pa.read_csv(self._in_data_dir("src_v2.csv"), header=0)
        df.rename(
            columns={
                "Sub type": "sub_type",
                "J3": "ra_hms",
                "J4": "ra_degrees",
                "J5": "dec_dms",
                "J6": "dec_degrees",
                "J7": "constilation",
                "J8": "nickname",
                "J9": "Alt_id",
                "J10": "Nearby_obj",
                "J11": "visual_mag",
                "J12": "surface_brightness",
                "J13": "inclination_degrees",
            },
            inplace=True,
        )
        df.drop(columns=["J1", "J2"], inplace=True)
        df["Type"] = df["Type"].str.strip()
        df["sub_type"] = df["sub_type"].str.strip()
        df["ks_type"] = self.ks_catagory(df)
        self._state = df
        # for _, obj in self._state.iterrows():
        for i, (_, obj) in enumerate(self._state.iterrows()):
            yield self._make_catalog_object(
                type=int(obj["ks_type"]),
                ra=float(obj["ra_degrees"]),
                dec=float(obj["dec_degrees"]),
                magnitude=float(obj["visual_mag"]),
                name=str(f"GI {i}"),
                long_name=str(obj["Name"]),
                catalog_identifier=str(f"GI {i}"),
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
            tmp_id = f"GI {i}"
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
