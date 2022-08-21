#include "puf.h"

#include <stdlib.h>

/*
 * Prototypes for private methods
 */

static int pufOn(puf *o, int eventName, pufcb callback);

static int pufEmit(puf *o, int eventName, uint8_t *param);

static int pufOff(puf *o, int eventName, pufcb callback);

static int pufFreeEventIndex(puf *o);

static int pufFreeCbIndex(puf *o, int eventIndex);

static int PufFreeEventAndCbIndex(puf *o, uint8_t eventName, int *eventIndex,
                                  int *eventCallback);

static int pufFindEventIndex(puf *o, uint8_t eventName);

static int pufFindCbIndex(puf *o, int eventIndex, pufcb callback);

static uint8_t pufFindEventAndCountCallbacks(puf *o, uint8_t eventName,
                                             int *eventIndex);

static int pufCountEventCallback(puf *o, int eventIndex);

static int pufRun(puf *o, int eventIndex, uint8_t *param);

static int pufReset(puf *o);

static int pufResetEvent(puf *o, int eventIndex);

/*
 * Definitions for public methods
 */

puf *puf_new(uint16_t sizeEvents, uint16_t sizeCallbacks) {
  puf *o = (puf *)malloc(sizeof(puf));

  o->sizeEvents = sizeEvents;
  o->sizeCallbacks = sizeCallbacks;

  // get memory for events and callbacks containers
  o->events = (int *)calloc(sizeEvents, sizeof(int));
  o->callbacks = (pufcb**)malloc(sizeEvents * sizeof(int *));
  for (uint16_t i = 0; i < sizeEvents; i++) {
    o->callbacks[i] = (pufcb*)malloc(sizeCallbacks * sizeof(pufcb));
  }

  // set public functions
  o->on = pufOn;
  o->emit = pufEmit;
  o->off = pufOff;
  o->reset = pufReset;
  o->resetEvent = pufResetEvent;

  o->reset(o);

  return o;
}

void puf_destroy(puf *o) {
  free(o->events);
  for (int i = 0; i < o->sizeEvents; i++) {
    free(o->callbacks[i]);
  }
  free(o->callbacks);
  free(o);
}

/*
 * Definitions for private methods
 */

static int pufOn(puf *o, int eventName, pufcb callback) {
  int nextEvent, nextCallback = 0;
  int err = PufFreeEventAndCbIndex(o, eventName, &nextEvent, &nextCallback);
  if (err != 0) {
    return err;
  }

  int callbackExists = pufFindCbIndex(o, nextEvent, callback);
  if (callbackExists != -1) {
    return 50;
  }

  o->events[nextEvent] = eventName;
  o->callbacks[nextEvent][nextCallback] = callback;

  return 0;
}

static int pufEmit(puf *o, int eventName, uint8_t *param) {
  int eventIndex = 0;
  uint8_t err = pufFindEventAndCountCallbacks(o, eventName, &eventIndex);
  return err ? err : pufRun(o, eventIndex, param);
}

static int pufOff(puf *o, int eventName, const pufcb callback) {
  int eventIndex = pufFindEventIndex(o, eventName);
  if (eventIndex == -1) {
    return 30;
  }

  for (int i = 0; i < o->sizeCallbacks; i++) {
    if (o->callbacks[eventIndex][i] == callback) {
      o->callbacks[eventIndex][i] = NULL;
      return i;
    }
  }

  return 31;
}

static int pufFreeEventIndex(puf *o) {
  for (int i = 0; i < o->sizeEvents; i++) {
    if (o->events[i] == 0) {
      return i;
    }
  }

  return -1;
}

static int pufFreeCbIndex(puf *o, int eventIndex) {
  for (int i = 0; i < o->sizeCallbacks; i++) {
    if (o->callbacks[eventIndex][i] == NULL) {
      return i;
    }
  }

  return -1;
}

static int pufFindEventIndex(puf *o, uint8_t eventName) {
  for (int i = 0; i < o->sizeEvents; i++) {
    if (o->events[i] == eventName) {
      return i;
    }
  }

  return -1;
}

static int pufFindCbIndex(puf *o, int eventIndex, const pufcb callback) {
  for (int i = 0; i < o->sizeCallbacks; i++) {
    if (o->callbacks[eventIndex][i] == callback) {
      return i;
    }
  }

  return -1;
}

static uint8_t pufFindEventAndCountCallbacks(puf *o, uint8_t eventName,
                                             int *eventIndex) {
  *eventIndex = pufFindEventIndex(o, eventName);
  if (*eventIndex == -1) {
    return 10;
  }

  int countCallbacks = pufCountEventCallback(o, *eventIndex);
  if (countCallbacks == 0) {
    return 11;
  }

  return 0;
}

static int PufFreeEventAndCbIndex(puf *o, uint8_t eventName, int *eventIndex,
                                  int *eventCallback) {
  *eventIndex = pufFindEventIndex(o, eventName);
  if (*eventIndex == -1) {
    *eventIndex = pufFreeEventIndex(o);
    if (*eventIndex == -1) {
      return 20;
    }
  }

  *eventCallback = pufFreeCbIndex(o, *eventIndex);
  if (*eventCallback == -1) {
    return 21;
  }

  return 0;
}

static int pufCountEventCallback(puf *o, int eventIndex) {
  int count = 0;
  for (int i = 0; i < o->sizeCallbacks; i++) {
    if (o->callbacks[eventIndex][i] != NULL) {
      count++;
    }
  }

  return count;
}

static int pufRun(puf *o, int eventIndex, uint8_t *param) {
  int err = 0;
  for (int i = 0; i < o->sizeCallbacks; i++) {
    if (o->callbacks[eventIndex][i] != NULL) {
      err = o->callbacks[eventIndex][i](param);
      if (err != 0) {
        break;
      }
    }
  }

  return err;
}

static int pufReset(puf *o) {
  for (int i = 0; i < o->sizeEvents; i++) {
    o->events[i] = 0;
    for (int j = 0; j < o->sizeCallbacks; j++) {
      o->callbacks[i][j] = NULL;
    }
  }

  return 0;
}

static int pufResetEvent(puf *o, int eventName) {
  int eventIndex = pufFindEventIndex(o, eventName);
  for (int i = 0; i < o->sizeCallbacks; i++) {
    o->callbacks[eventIndex][i] = NULL;
  }

  return -1;
}
