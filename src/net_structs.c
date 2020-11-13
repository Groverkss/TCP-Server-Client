#include <stdint.h>
#include "net_structs.h"
#include <string.h>

void init_pay(struct Payload *payload) {
    /* Fix byte order. Convert order and shit */
    memset(payload, 0, sizeof(struct Payload));
    payload->response = 3;
    payload->call = 0;
    payload->length = 0;
    payload->left = 0;
    bzero(payload->data, RESPONSE_SIZE);
}
