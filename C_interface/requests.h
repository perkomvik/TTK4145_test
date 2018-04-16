#pragma once

#include "elevator.h"
#include "elevator_io_types.h"


Dirn requests_chooseDirection(Elevator elev) __attribute__((pure));

int requests_shouldStop(Elevator elev) __attribute__((pure));
int requests_atCurrentFloor(Elevator elev) __attribute__((pure));

Elevator requests_clearAtCurrentFloor(Elevator elev) __attribute__((pure));
