# File: quine_mccluskey.py
# Description: Quine-McCluskey algorithm implementation

from typing import List


class Implicant:
    """Implementation of a single implicant occurring in groups in the algorithm."""
    def __init__(self, group: int, minterms: List[int], binary: str):
        self.group = group
        self.minterms = minterms
        self.binary = binary

    def can_merge(self, other: "Implicant") -> int:
        """Check if two implicants can be merged.

        Args:
            other (Implicant): Implicant to be merged with

        Returns:
            int: Index of the bit that differs between the two implicants, -1 if they cannot be merged
        """
        # adjust binary representation to be of the same length
        len_self, len_other = len(self.binary), len(other.binary)
        if len_self > len_other:
            other.binary = other.binary.rjust(len_self, "0")
        elif len_self < len_other:
            self.binary = self.binary.rjust(len_other, "0")

        # check if implicants can be merged -- they differ by only one bit
        merge_index = -1
        index = -1
        for implicant_a, implicant_b in zip(self.binary, other.binary):
            index += 1

            if implicant_a != implicant_b:
                if merge_index >= 0:
                    # second difference found, cannot merge
                    return -1
                merge_index = index

        return merge_index

    def merge(self, other: "Implicant", index: int) -> "Implicant":
        """Merge two implicants.

        Args:
            other (Implicant): Implicant to be merged with
            index (int): Index of the bit that differs between the two implicants

        Returns:
            Implicant: Merged implicant
        """
        minterms = self.minterms + other.minterms
        binary = self.binary[:index] + "-" + self.binary[index + 1:]
        group = binary.count("1")

        return Implicant(group=group, minterms=minterms, binary=binary)

    def __str__(self) -> str:
        """String representation of the implicant.

        Examples:
            >>> implicant = Implicant(group=2, minterms=[0, 1, 2, 3], binary="011-")
            >>> print(implicant)
            a' b c

        Returns:
            str: String representation of the implicant
        """
        # 0 1 - 0 = a' * b * d'

        str_parts = []
        for index, bit in enumerate(self.binary):
            if bit == "0":
                str_parts.append(f"{chr(97 + index)}'")
            elif bit == "1":
                str_parts.append(f"{chr(97 + index)}")

        return " ".join(str_parts)


def merge_groups(group_a: list, group_b: list) -> dict:
    """Merge two groups of implicants.

    Args:
        group_a (list): First group of implicants
        group_b (list): Second group of implicants

    Returns:
        dict: New groups of implicants created by merging the two groups
    """
    subgroup = {}
    for implicant_a in group_a:
        for implicant_b in group_b:
            merge_index = implicant_a.can_merge(implicant_b)
            if merge_index >= 0:
                merged_implicant = implicant_a.merge(implicant_b, merge_index)

                if merged_implicant.group not in subgroup:
                    subgroup[merged_implicant.group] = []
                subgroup[merged_implicant.group].append(merged_implicant)

    return subgroup


def quine_mccluskey(bits: int, input_list: List[int]):
    """Quine-McCluskey algorithm implementation.

    Algorithm for minimising boolean functions.

    Args:
        bits (int): Number of bits in the binary representation of the function
        input_list (List[int]): Shortened normal disjunctive form of the boolean function

    Returns:
        str: Minimised normal disjunctive form of the boolean function
    """
    groups = {}

    # group implicants by number of ones in their binary representation
    for implicant in input_list:
        bin_implicant = bin(implicant)[2:].rjust(bits, "0")
        if (one_count := bin_implicant.count('1')) not in groups:
            groups[one_count] = []
        groups[one_count].append(Implicant(group=one_count, minterms=[implicant], binary=bin_implicant))

    # merge groups until no more merging is possible
    while True:
        new_groups = {}
        keys = sorted(groups.keys())
        for i in range(len(keys) - 1):
            subgroup = merge_groups(groups[keys[i]], groups[keys[i + 1]])
            for key in subgroup.keys():
                if key not in new_groups:
                    new_groups[key] = []
                new_groups[key] += subgroup[key]
        if new_groups:
            groups = new_groups
        else:
            break

    # filter out duplicate implicants
    filtered_implicants = []
    for group in groups.values():
        for i, implicant in enumerate(group):
            for j in range(i + 1, len(group)):
                if set(implicant.minterms) == set(group[j].minterms):
                    break
            else:
                filtered_implicants.append(implicant)

    # select implicants with unique minterms
    unique_implicants = []
    for i, implicant_a in enumerate(filtered_implicants):
        minterms = set(implicant_a.minterms)
        for j, implicant_b in enumerate(filtered_implicants):
            if i == j:
                continue
            minterms -= set(implicant_b.minterms)
        if minterms:
            unique_implicants.append(implicant_a)

    # print result
    function_parts = [implicant.__str__() for implicant in unique_implicants]
    print((
        f"F({', '.join(chr(char) for char in range(97, 97 + bits))}) = "
        + f" SUM m ({', '.join([str(minterm) for minterm in input_list])}) = "
        + " + ".join(function_parts)
    ))


if __name__ == "__main__":
    bits, input_list = 4, [0, 1, 3, 7, 8, 9, 11, 15]
    # bits, input_list = 4, [0, 1, 2, 4, 6, 8, 9, 11, 13, 15]
    # bits, input_list = 7, [20, 28, 52, 60]
    quine_mccluskey(bits, input_list)
