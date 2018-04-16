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
    elevator.config.doorOpenDuration_s = 3
}

static void setAllLights(Elevator es){
    for(int floor = 0; floor < N_FLOORS; floor++){
        for(int btn = 0; btn < N_BUTTONS; btn++){
            elevator_hardware_set_button_lamp(btn, floor, es.requests[floor][btn]);
        }
    }
}

void fsm_onInitBetweenFloors(void){
    elevator_hardware_set_motor_direction(D_Down);
    elevator.dirn = D_Down;
    elevator.behaviour = EB_Moving;
}

void fsm_onRequestButtonPress(int btn_floor, Button btn_type){
    //printf("spam\n");
    switch(elevator.behaviour){

    case EB_DoorOpen:
        if(elevator.floor == btn_floor){
              //printf("1\n");
            //if (requests_shouldStop(elevator)){
              //fsm_clear_floor(btn_floor);
              if (btn_type != 2){
                elevator.requests[btn_floor][btn_type] = 1;
                //elevator_hardware_set_door_open_lamp(1);
              }
              //printf("hei\n");
              timer_start(elevator.config.doorOpenDuration_s);
              //setAllLights(elevator);
              //return 1;
            //}

        } else {
            //printf("5\n");
            elevator.requests[btn_floor][btn_type] = 1;
            if (elevator_hardware_get_floor_sensor_signal() == btn_floor){
                //printf("6\n");
                timer_start(elevator.config.doorOpenDuration_s);
            }
        }
        break;

    case EB_Moving:
        //printf("2\n");
        //if (elevator_hardware_get_floor_sensor_signal() == btn_floor){
            //fsm_onFloorArrival(btn_floor);
        //}
        elevator.requests[btn_floor][btn_type] = 1;
        break;

    case EB_Idle:
        if(elevator.floor == btn_floor){
            //printf("3\n");
            elevator_hardware_set_door_open_lamp(1);
            timer_start(elevator.config.doorOpenDuration_s);
            elevator.behaviour = EB_DoorOpen;
            //elevator.requests[btn_floor][btn_type] = 1;
        } else {
            //printf("4\n");
            elevator.requests[btn_floor][btn_type] = 1;
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
        } else {
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

int fsm_get_e_request(int floor, int btn){
    return elevator.requests[floor][btn];
}

int fsm_set_e_request(int value, int floor, int btn){
    elevator.requests[floor][btn] = value;
}

void fsm_clear_floor(int floor){
    for(Button btn = 0; btn < N_BUTTONS-1; btn++){
         elevator.requests[floor][btn] = 0;
    }
    setAllLights(elevator);
}
