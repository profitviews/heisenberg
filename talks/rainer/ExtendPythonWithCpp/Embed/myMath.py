print("Python: ", end = "")

def fakul(num):
    from functools import reduce
    print("Returning fakul({})".format(num))
    return reduce(lambda x, y: x * y, range(1, num + 1))

def sum(fir, sec):
    print("Returning sum({}, {})".format(fir, sec))
    return fir + sec

def product(fir, sec):
    print("Returning product({}, {})".format(fir, sec))
    return fir * sec
