x=2
for _ in range(21):
    x *= x
print ("21 times x*=x, xi=2 has %s digits" % (len(str(x)),))
