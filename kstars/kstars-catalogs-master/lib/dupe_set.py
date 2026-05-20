from typing import Set, Tuple, List, Iterable

DupeSet = Set[Tuple[int, bytes]]


class DupeContainter:
    _storage: List[DupeSet] = []

    def add(self, dupes: DupeSet) -> None:
        for i, item in enumerate(self._storage):
            if dupes & item:
                self._storage[i] = item | dupes
                return

        self._storage.append(dupes)

    def add_many(self, dupes: Iterable[DupeSet]) -> None:
        for dupe in dupes:
            self.add(dupe)

    @property
    def dupes(self) -> List[DupeSet]:
        return self._storage
