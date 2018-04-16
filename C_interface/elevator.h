#pragma once

#include "elevator_io_types.h"

typedef enum {
    EB_Idle,
    EB_DoorOpen,
    EB_Moving
} ElevatorBehaviour;

typedef struct {
    int                     floor;
    Dirn                    dirn;
    int                     requests[N_FLOORS][N_BUTTONS];
    ElevatorBehaviour       behaviour;

    struct {
        double              doorOpenDuration_s;
    } config;
} Elevator;


void elevator_print(Elevator es);

Elevator elevator_uninitialized(void);
