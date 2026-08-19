#ifndef SLIMEWORKS_INPUT_GATE_H
#define SLIMEWORKS_INPUT_GATE_H

#include <stdbool.h>

typedef struct InputGate {
    bool gameplayBlocked;
} InputGate;

static inline void InputGateInit(InputGate *gate) { *gate = (InputGate){ 0 }; }

#endif
