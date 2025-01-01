import sys


def convert_vowel(x):
    return [
        "_",
        "k",
        "s",
        "t",
        "n",
        "h",
        "m",
        "y",
        "r",
        "w",
        "nn",
    ][x - 1]


def convert_consonant(x):
    return [
        "a",
        "i",
        "u",
        "e",
        "o",
    ][x - 1]


def convert_letter(v, c):
    if v == 11 and c == 1:
        return "nn"
    return convert_vowel(v) + "-" + convert_consonant(c)


l = sys.stdin.read()
print(l)
xs = l.split(",")
for x in xs:
    vowel_str, consonant_str = x.split(".")
    vowel = int(vowel_str)
    consonant = int(consonant_str)
    print(f"{vowel:2} {consonant} => {convert_letter(vowel, consonant)}")
