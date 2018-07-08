# Instructions

+ LDNUM \<number\> \<reg\>
    - Load literal number into reg
+ LDBOOL \<true|false\> \<reg\>
    - Load literal boolean into reg
+ LDSTR \<citron\_string\> \<reg\>
    - Load string into register
+ LDNAME \<name\> \<reg\>
    - Load object by name into register
+ STORE \<reg\> \<name\>
    - Store the object in reg as name
+ UMESG \<reg\> \<msg\> [\<resreg\>]
    - Send unary message msg to the object in `reg`, place the result in resreg if specified
+ BMESG \<reg\> \<msg\> \<arg\_reg\> [\<resreg\>]
    - Send a binary message to the object in `reg`
+ KMESG \<reg\> \<msg\> {\<arg\_reg\>} [\<resreg\>]
    - Send a keyword message to the object in `reg`

