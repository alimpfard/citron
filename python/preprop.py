import re
from ctypes import cdll, c_char_p

ctr = cdll.LoadLibrary('./libctr.so')
ctr.initialize()
ctr.execute_string.argtypes = [c_char_p]
ctr.execute_string.restype = c_char_p
def ctr_exec(string):
    if string:
        print ("Ctr <- " + string)
        return ctr.execute_string(string.encode('utf-8'))
    return ''

if __name__ == '__main__':
    print ("Magic happenning now!")
    while True:
        string = ('(' + input(">>> ").trim() + ')')
        print('Ctr -> ' + (ctr_exec(string).decode('utf-8')))
