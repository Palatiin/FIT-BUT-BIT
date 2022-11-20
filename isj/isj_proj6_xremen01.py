#!/usr/bin/env python3


class Polynomial:
    """Implementation of polynom representation and operations with it.
    Attributes:
        polynom: dict (key/exponent : value)
    Methods: __init__, __str__, __eq__, __add__, __pow__, derivative, at_value
    """
    def __init__(self, *args, **kwargs):
        """Initialize polynom."""
        self.polynom = {0: 0}
        if args:
            unified = args[0] if isinstance(args[0], list) else list(args)
            self.polynom = {i: v for i, v in enumerate(unified) if v != 0}
        elif kwargs:
            self.polynom = {int(k[1:]): v for k, v in kwargs.items() if v != 0}

    def __str__(self):
        """String representation of object."""
        return (" ".join([
                "{}{}{}{}".format(
                    "+ " if v > 0 else "- ",
                    abs(v) if abs(v) > 1 or k == 0 else "",
                    "x" if k else "",
                    "^" + str(k) if k > 1 else ""
                )
                for k, v in sorted(self.polynom.items(), reverse=True)
            ]).strip(" +-")
            or "0"
        )

    def __eq__(self, other):
        """Check if polynoms are equal.

        Args:
            other: second operand
        """
        return self.__class__ == other.__class__ and self.polynom == other.polynom

    def __add__(self, other):
        """Sum up two polynoms.

        Args:
            other: second operand
        
        Return:
            type Polynomial, self + other
        """
        return Polynomial(**{
            f"x{key}": self.polynom.get(key, 0) + other.polynom.get(key, 0)
            for key in set(self.polynom.keys()) | set(other.polynom.keys())
        })

    def __pow__(self, power):
        """Power the polynom.

        Args:
            power: exponent
        
        Return:
            type Polynomial, (polynom)^power
        """
        part = self.polynom if power else {0: 1}
        for _ in range(power - 1):
            final = {}
            for key1, val1 in self.polynom.items():
                for key2, val2 in part.items():
                    final[key1 + key2] = final.get(key1 + key2, 0) + val1 * val2
            part = final
        return Polynomial(**{f"x{k}": v for k, v in part.items()})

    def derivative(self):
        """Perform first derivation on plynom."""
        return Polynomial(**{f"x{k-1}": v * k for k, v in self.polynom.items() if k > 0} or {"x0": 0})

    def at_value(self, first, second=None):
        """Check value of polynom at specific value.

        Args:
            first: first value of 'x'
            second: optional second value of 'x'

        Return:
            type Polynomial, value of polynom at value 'first'
            or value at value 'second' minus value at 'first'
        """
        if second:
            return self.at_value(second) - self.at_value(first)

        return sum([v * first ** k for k, v in self.polynom.items()])


def test():
    assert str(Polynomial(0,1,0,-1,4,-2,0,1,3,0)) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x"
    assert str(Polynomial([-5,1,0,-1,4,-2,0,1,3,0])) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x - 5"
    assert str(Polynomial(x7=1, x4=4, x8=3, x9=0, x0=0, x5=-2, x3= -1, x1=1)) == "3x^8 + x^7 - 2x^5 + 4x^4 - x^3 + x"
    assert str(Polynomial(x2=0)) == "0"
    assert str(Polynomial(x0=0)) == "0"
    assert Polynomial(x0=2, x1=0, x3=0, x2=3) == Polynomial(2,0,3)
    assert Polynomial(x2=0) == Polynomial(x0=0)
    assert str(Polynomial(x0=1)+Polynomial(x1=1)) == "x + 1"
    assert str(Polynomial([-1,1,1,0])+Polynomial(1,-1,1)) == "2x^2"
    pol1 = Polynomial(x2=3, x0=1)
    pol2 = Polynomial(x1=1, x3=0)
    assert str(pol1+pol2) == "3x^2 + x + 1"
    assert str(pol1+pol2) == "3x^2 + x + 1"
    assert str(Polynomial(x0=-1,x1=1)**1) == "x - 1"
    assert str(Polynomial(x0=-1,x1=1)**2) == "x^2 - 2x + 1"
    pol3 = Polynomial(x0=-1,x1=1)
    assert str(pol3**4) == "x^4 - 4x^3 + 6x^2 - 4x + 1"
    assert str(pol3**4) == "x^4 - 4x^3 + 6x^2 - 4x + 1"
    assert str(Polynomial(x0=2).derivative()) == "0"
    assert str(Polynomial(x3=2,x1=3,x0=2).derivative()) == "6x^2 + 3"
    assert str(Polynomial(x3=2,x1=3,x0=2).derivative().derivative()) == "12x"
    pol4 = Polynomial(x3=2,x1=3,x0=2)
    assert str(pol4.derivative()) == "6x^2 + 3"
    assert str(pol4.derivative()) == "6x^2 + 3"
    assert Polynomial(-2,3,4,-5).at_value(0) == -2
    assert Polynomial(x2=3, x0=-1, x1=-2).at_value(3) == 20
    assert Polynomial(x2=3, x0=-1, x1=-2).at_value(3,5) == 44
    pol5 = Polynomial([1,0,-2])
    assert pol5.at_value(-2.4) == -10.52
    assert pol5.at_value(-2.4) == -10.52
    assert pol5.at_value(-1,3.6) == -23.92
    assert pol5.at_value(-1,3.6) == -23.92

if __name__ == '__main__':
    test()
