import re
from ctypes import cdll, c_char_p, RTLD_GLOBAL
import ctypes

ctr = ctypes.CDLL('./libctr.so', mode = RTLD_GLOBAL)
ctr.initialize(1)
ctr.execute_string.argtypes = [c_char_p]
ctr.execute_string.restype = c_char_p
def ctr_exec(string):
    if string:
        print ("Ctr <- " + string)
        return ctr.execute_string(string.encode('utf-8'))
    return None

if __name__ == '__main__':
    print ("Magic happenning now!")
    while True:
        string = (input(">>> ").strip())
        print('Ctr -> ' + ((ctr_exec(string) or b'').decode('utf-8')))
