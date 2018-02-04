#include <stdio.h>
#include <stddef.h>

typedef struct _libwebsock_message {
        unsigned int opcode;
        unsigned long long payload_len;
        char *payload;
} libwebsock_message;


int main() {
    printf("size %d\nopcode offset %d size %d\n", sizeof(libwebsock_message),offsetof(libwebsock_message, opcode), sizeof(unsigned int));
    printf("payloadlen offset %d size %d\n", offsetof(libwebsock_message, payload_len), sizeof(unsigned long long));
    printf("payload offset %d\n", offsetof(libwebsock_message, payload));

}
