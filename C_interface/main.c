
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "con_load.h"
//#include "elevator_io_device.h"
#include "driver/elevator_hardware.h"
#include "fsm.h"
#include "timer.h"


int main(void){

    int inputPollRate_ms = 25;
    elevator_hardware_init();

    if(elevator_hardware_get_floor_sensor_signal() == -1){
        fsm_onInitBetweenFloors();
    }

    while(1){
        { // Request button
            static int prev_button_status[N_FLOORS][N_BUTTONS];
            for(int floor = 0; floor < N_FLOORS; floor++){
                for(int button = 0; button < N_BUTTONS; button++){
                    int button_status = elevator_hardware_get_button_signal(button, floor);
                    if(button_status &&  button_status != prev_button_status[floor][button]){
                        fsm_onRequestButtonPress(floor, button);
                    }
                    prev_button_status[floor][button] = button_status;
                }
            }
        }

        { // Floor sensor
            static int prev_floor;
            int floor = elevator_hardware_get_floor_sensor_signal();
            if(floor != -1  &&  floor != prev_floor){
                fsm_onFloorArrival(floor);
            }
            prev_floor = floor;
        }


        { // Timer
            if(timer_timedOut()){
                fsm_onDoorTimeout();
                timer_stop();
            }
        }

        usleep(inputPollRate_ms*1000);
    }
}
