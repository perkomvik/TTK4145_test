#include "requests.h"

static int requests_above(Elevator elev){
    for(int floor = elev.floor+1; floor < N_FLOORS; floor++){
        for(int button = 0; button < N_BUTTONS; button++){
            if(elev.requests[floot][button]){
                return 1;
            }
        }
    }
    return 0;
}

static int requests_below(Elevator elev){
    for(int floor = 0; floor < elev.floor; floor++){
        for(int button = 0; button < N_BUTTONS; button++){
            if(elev.requests[floor][button]){
                return 1;
            }
        }
    }
    return 0;
}

Dirn requests_chooseDirection(Elevator elev){
    switch(elev.dirn){
    case D_Up:
        return  requests_above(elev) ? D_Up    :
                requests_below(elev) ? D_Down  :
                                    D_Stop  ;
    case D_Down:
    case D_Stop: // there should only be one request in this case. Checking up or down first is arbitrary.
        return  requests_below(elev) ? D_Down  :
                requests_above(elev) ? D_Up    :
                                    D_Stop  ;
    default:
        return D_Stop;
    }
}

int requests_shouldStop(Elevator elev){
    switch(elev.dirn){
    case D_Down:
        return
            elev.requests[elev.floor][B_HallDown] ||
            elev.requests[elev.floor][B_Cab]      ||
            !requests_below(elev);
    case D_Up:
        return
            elev.requests[elev.floor][B_HallUp]   ||
            elev.requests[elev.floor][B_Cab]      ||
            !requests_above(elev);
    case D_Stop:

    default:
        return 1;
    }
}
int requests_atCurrentFloor(Elevator elev){
      return
          elev.requests[elev.floor][B_HallUp]   ||
          elev.requests[elev.floor][B_HallDown] ||
          elev.requests[elev.floor][B_Cab];
}


//Elevator requests_clearAtCurrentFloor(Elevator e){
Elevator requests_clearAtCurrentFloor(Elevator elev){
    elev.requests[elev.floor][B_Cab] = 0;
        switch(elev.dirn){
        case D_Up:
            elev.requests[elev.floor][B_HallUp] = 0;
            if(!requests_above(elev)){
                elev.requests[elev.floor][B_HallDown] = 0;
            }
            break;

        case D_Down:
            elev.requests[elev.floor][B_HallDown] = 0;
            if(!requests_below(elev)){
                elev.requests[elev.floor][B_HallUp] = 0;
            }
            break;

        case D_Stop:
        default:
            elev.requests[elev.floor][B_HallUp] = 0;
            elev.requests[elev.floor][B_HallDown] = 0;
            break;
        }
    return elev;
}
