"""A random catalog generator to test the catalog factory"""

from lib.catalogfactory import Factory, Catalog
import random
import string
from pykstars import ObjectType


def generate_random_string(str_size, allowed_chars=string.ascii_letters):
    return "".join(random.choice(allowed_chars) for x in range(str_size))


class RandomCatalogBase(Factory):
    SIZE = 100

    def load_objects(self):
        i = 0
        for _ in range(self.SIZE):
            ++i
            ob_type = random.choice(
                [ObjectType.STAR, ObjectType.GALAXY, ObjectType.GASEOUS_NEBULA]
            )
            ra = random.uniform(0, 360)
            dec = random.uniform(-90, 90)
            mag = random.uniform(4, 16)
            name = generate_random_string(5)
            long_name = generate_random_string(10)
            print(f"Object {i} output")

            yield self._make_catalog_object(
                type=ob_type,
                ra=ra,
                dec=dec,
                magnitude=mag,
                name=name,
                long_name=long_name,
                position_angle=random.uniform(0, 180),
                flux=random.uniform(0, 10),
            )


class HugeRandomCatalog(RandomCatalogBase):
    meta = Catalog(
        id=999,
        name="random",
        maintainer="Valentin Boettcher <hiro@protagon.space>",
        license="DWYW Do what ever you want with it!",
        description="A huge catalog of random DSOs",
        precedence=1,
        version=1,
    )

    SIZE = 1000_000
