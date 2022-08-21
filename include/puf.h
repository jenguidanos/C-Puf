#pragma once

#include <stdint.h>
#include <string.h>

typedef struct puf puf;
typedef int (*pufcb)(uint8_t * param);

#define PUF_ARG(str) (uint8_t*) &str
#define PUF_ARG_P(str) (uint8_t*) str
#define PUF_READ_INT(varName, data) memcpy(&varName, data, sizeof(varName));
#define PUF_READ_CHAR(varName, data, size) memcpy(&varName, data, size);

struct puf {
  int sizeEvents;
  int sizeCallbacks;
  int * events;
  pufcb ** callbacks;

  int (*on)(puf * o, int eventName, pufcb callback);
  int (*emit)(puf * o, int eventName, uint8_t *param);
  int (*off)(puf * o, int eventName, pufcb callback);
  int (*reset)(puf * o);
  int (*resetEvent)(puf * o, int eventName);
};

puf * puf_new(uint16_t sizeEvents, uint16_t sizeCallbacks);

void puf_destroy(puf* o);
