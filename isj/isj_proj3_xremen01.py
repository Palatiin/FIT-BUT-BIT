#!/usr/bin/env python3

# ukol za 2 body
def first_odd_or_even(numbers):
    """Returns 0 if there is the same number of even numbers and odd numbers
       in the input list of ints, or there are only odd or only even numbers.
       Returns the first odd number in the input list if the list has more even
       numbers.
       Returns the first even number in the input list if the list has more odd 
       numbers.

    >>> first_odd_or_even([2,4,2,3,6])
    3
    >>> first_odd_or_even([3,5,4])
    4
    >>> first_odd_or_even([2,4,3,5])
    0
    >>> first_odd_or_even([2,4])
    0
    >>> first_odd_or_even([3])
    0
    """
    science = {
        "evens": {
            "count": 0,
            "first": None,
        },
        "odds": {
            "count": 0,
            "first": None,
        }
    }
    for num in numbers:
        if num % 2:
            science["odds"]["count"] += 1
            science["odds"]["first"] = science["odds"]["first"] if science["odds"]["first"] else num
        else:
            science["evens"]["count"] += 1
            science["evens"]["first"] = science["odds"]["first"] if science["evens"]["first"] else num
    
    if (
        science["odds"]["count"] == science["evens"]["count"]
        or not science["odds"]["count"]
        or not science["evens"]["count"]
    ):
        return 0
    elif science["evens"]["count"] > science["odds"]["count"]:
        return science["odds"]["first"]
    return science["evens"]["first"]


# ukol za 3 body
def to_pilot_alpha(word):
    """Returns a list of pilot alpha codes corresponding to the input word

    >>> to_pilot_alpha('Smrz')
    ['Sierra', 'Mike', 'Romeo', 'Zulu']
    """

    pilot_alpha = ['Alfa', 'Bravo', 'Charlie', 'Delta', 'Echo', 'Foxtrot',
        'Golf', 'Hotel', 'India', 'Juliett', 'Kilo', 'Lima', 'Mike',
        'November', 'Oscar', 'Papa', 'Quebec', 'Romeo', 'Sierra', 'Tango',
        'Uniform', 'Victor', 'Whiskey', 'Xray', 'Yankee', 'Zulu']

    word = word.lower()
    pilot_alpha_list = [pilot_alpha[ord(char)-97] for char in word]
     
    return pilot_alpha_list


if __name__ == "__main__":
    import doctest
    doctest.testmod()
