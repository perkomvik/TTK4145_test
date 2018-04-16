#include "fsm.h"

#include <stdio.h>

#include "con_load.h"
#include "elevator.h"
#include "driver/elevator_hardware.h"
#include "requests.h"
#include "timer.h"

static Elevator             elevator;
static ElevOutputDevice     outputDevice;


static void __attribute__((constructor)) fsm_init(){
    elevator = elevator_uninitialized();
}

static void setAllLights(Elevator elev){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int button = 0; button < N_BUTTONS; button++){
            elevator_hardware_set_button_lamp(button, floor, elev.requests[floor][button]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
    elevator_hardware_set_motor_direction(D_Down);
    elevator.dirn = D_Down;
    elevator.behaviour = EB_Moving;
}

void fsm_onRequestButtonPress(int button_floor, Button button_type){
    switch(elevator.behaviour){

    case EB_DoorOpen:
        if(elevator.floor == button_floor){
            if (button_type != 2){
              elevator.requests[button_floor][button_type] = 1;
            }
            timer_start(elevator.config.doorOpenDuration_s);

        }
        else {
            elevator.requests[button_floor][button_type] = 1;
            if (elevator_hardware_get_floor_sensor_signal() == button_floor){
                timer_start(elevator.config.doorOpenDuration_s);
            }
        }
        break;

    case EB_Moving:
        elevator.requests[button_floor][button_type] = 1;
        break;

    case EB_Idle:
        if(elevator.floor == button_floor){
            elevator_hardware_set_door_open_lamp(1);
            timer_start(elevator.config.doorOpenDuration_s);
            elevator.behaviour = EB_DoorOpen;
        }
        else {
            elevator.requests[button_floor][button_type] = 1;
            elevator.dirn = requests_chooseDirection(elevator);
            elevator_hardware_set_motor_direction(elevator.dirn);
            elevator.behaviour = EB_Moving;
        }
        break;
    }
    setAllLights(elevator);
}




int fsm_onFloorArrival(int newFloor){
    elevator.floor = newFloor;
    elevator_hardware_set_floor_indicator(elevator.floor);

    switch(elevator.behaviour){

    case EB_Moving:
        if(requests_shouldStop(elevator)){
            elevator_hardware_set_motor_direction(D_Stop);
            elevator_hardware_set_door_open_lamp(1);
            elevator = requests_clearAtCurrentFloor(elevator);
            timer_start(elevator.config.doorOpenDuration_s);
            setAllLights(elevator);
            elevator.behaviour = EB_DoorOpen;
            return 1;
        }
        return 0;

    case EB_DoorOpen:
        if(requests_shouldStop(elevator)){
            return 1;
        }
    default:
        return 0;
    }
}



void fsm_onDoorTimeout(void){
    switch(elevator.behaviour){

    case EB_DoorOpen:
        elevator.dirn = requests_chooseDirection(elevator);
        elevator_hardware_set_door_open_lamp(0);
        elevator_hardware_set_motor_direction(elevator.dirn);
        if(elevator.dirn == D_Stop){
            elevator.behaviour = EB_Idle;
        }
        else {
            elevator.behaviour = EB_Moving;
        }

        break;

    default:
        break;
    }

}

int fsm_get_e_floor(void){
    return elevator.floor;
}

int fsm_get_e_dirn(void){
    return elevator.dirn;
}

int fsm_get_e_behaviour(void){
    return elevator.behaviour;
}

int fsm_get_e_request(int floor, int button){
    return elevator.requests[floor][button];
}

int fsm_set_e_request(int value, int floor, int button){
    elevator.requests[floor][button] = value;
}

void fsm_clear_floor(int floor){
    for(Button button = 0; button < N_BUTTONS-1; button++){
         elevator.requests[floor][button] = 0;
    }
    setAllLights(elevator);
}
