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
import math


class Herschel_400(Factory):
    meta = Catalog(
        id=12,
        name="Hershell 400",
        author="Brenda F. Guzman (Branchett), Lydel Guzman, Paul Jones, James Morris, Peggy Taylor and Sara Saey",
        description="""The Herschel 400 catalogue is a subset of William Herschel's original Catalogue of Nebulae and Clusters of Stars, selected by Brenda F. Guzman (Branchett), Lydel Guzman, Paul Jones, James Morris, Peggy Taylor and Sara Saey of the Ancient City Astronomy Club in St. Augustine, Florida, United States c. 1980. They decided to generate the list after reading a letter[1] published in Sky & Telescope by James Mullaney of Pittsburgh, Pennsylvania, United States.[2]""",
        source="<a href=''https://en.wikipedia.org/wiki/Herschel_400_Catalogue</a>",
        precedence=0.4,
        version=1,
        maintainer="Tim Seed<tim@sy-edm.com>",
        license="Free for non-commercial use only",
        colors={"default": "#dbffd5"},
        image="hershell_400.jpg",
    )
    records = []

    def load_objects(self):

        df = pa.read_csv(self._in_data_dir("Hers_400_full.csv"), sep=",")

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
            "G": KSTARS_TYPE_MAP["G"],
            "OCl": KSTARS_TYPE_MAP["OC"],
            "GCl": KSTARS_TYPE_MAP["GC"],
            "PN": KSTARS_TYPE_MAP["PN"],
            "Dup": KSTARS_TYPE_MAP["PN"],
            "Cl+N": KSTARS_TYPE_MAP["DIFFUSE"],
            "Neb": KSTARS_TYPE_MAP["GASEOUS"],
            "RfN": KSTARS_TYPE_MAP[
                "EMISSION"
            ],  # Should be Reflection but no kstars object
            "GPair": KSTARS_TYPE_MAP["CLUSTER"],  # Again a pair of galaxies edge on
            "HII": KSTARS_TYPE_MAP["EMISSION"],
        }
        df["ra_degrees"] = df.ra * 180.0 / math.pi
        df["dec_degrees"] = df["dec"].apply(math.degrees)
        print("Finished loading the Data")
        print(f"Columns are {df.columns}")
        df["KStars_Type"] = df["BRNtype"].map(mapping)
        self._state = df
        # for _, obj in self._state.iterrows():
        for i, (_, obj) in enumerate(self._state.iterrows()):
            yield self._make_catalog_object(
                type=int(obj["KStars_Type"]),
                ra=float(obj["ra_degrees"]),
                dec=float(obj["dec_degrees"]),
                magnitude=float(obj["cstarvmag"]),
                name=str(obj["commonnames"]),
                catalog_identifier=str(obj["H_id"]),
                major_axis=float(obj.majax),
                minor_axis=float(obj.minax),
                #    minor_axis=0.0,  # Not mentioned in Catalog source
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
                    self.meta.id, f"catalog_identifier = {obj['commonnames']}"
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
