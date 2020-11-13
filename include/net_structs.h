#ifndef _NET_STRUCTS_
#define _NET_STRUCTS_

#define RESPONSE_SIZE 8192

struct Payload {
    uint16_t response;
    uint16_t call;
    uint16_t length;
    uint64_t left;
    char data[RESPONSE_SIZE];
};

void init_pay(struct Payload *payload);

#endif

/**
 * Call Numbers:
 * 1 --> List files: ls
 * 2 --> Change Directory: cd
 * 3 --> Get files: get
 */

/** 
 * Response Numbers:
 * 0 --> Command
 * 1 --> More
 * 2 --> End
 * 3 --> Error
 */
