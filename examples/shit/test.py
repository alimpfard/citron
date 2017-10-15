a,b = 0,1
#try:
for i in range(0,10**5):
    b,a=a,b
    b = b+a
    print (f"i:${i}")
#except:
#    with open('fib%s.txt'%i, 'w') as f:
#        f.write(str(b))
print (b)
