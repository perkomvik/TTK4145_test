__author__      = "gitgudd"

from order_fulfillment import *
from network import *
from order_assignment import *
import subprocess
import argparse as ap
import getpass as gp


STATE_NONE = 0
STATE_SYNC_WORLDVIEW = 1
STATE_SYNC_ELEVATOR = 2
STATE_EXIT = 3
STATE_ELEVATOR_OOO = 4
STATE_LOST_PEERS = 5
STATE_IDLE = 6
STATE_ASSIGN = 7



class State_machine:
    def __init__(self, local_orders_queue, worldview_queue):
        self.id = None
        self.config_init()
        self.state = STATE_NONE
        self.worldview = worldview = {}
    	self.worldview['hall_orders'] = [[[0,0] for x in range(0,N_BUTTONS-1)] for y in range(0,N_FLOORS)]
    	self.worldview['elevators'] = {}
        self.peers = None
        self.lost_peers = None
        self.elevator = None
        self.alone = False
        self.local_orders_queue = local_orders_queue
        self.worldview_queue = worldview_queue
    def hardware_failure(self):
        return self.worldview['elevators'][self.id]['hardware_failure']

    def redundancy_check(self):
        if len(self.peers) < 2:
            for floor in range (0, N_FLOORS):
                    for button in range (0, N_BUTTONS-1):
                        self.worldview['elevators'][self.id]['requests'][floor][button] = 0
                        self.worldview['hall_orders'][floor][button] =[0,0]

    def set_elevator(self, elevator):
        self.elevator = elevator
        self.state = STATE_SYNC_ELEVATOR
        self.worldview['elevators'][self.id] = elevator
        #self.redundancy_check()



    def update_peers(self, peers, lost_peers):
        self.peers = peers
        self.lost_peers = lost_peers
        self.delete_lost_peers()

    def pass_local_worldview(self):
        #if len(self.peers) >= 2:
        local_orders = self.worldview['elevators'][self.id]['requests']
        self.local_orders_queue.join()
        self.local_orders_queue.put(local_orders)

    def pass_worldview(self):
        worldview_with_id = {}
        worldview_with_id[self.id] = self.worldview
        print(11)
        #self.worldview_queue.join()
        #if (self.worldview_queue.empty()):
            #worldview_queue.join()
            #print(12)
        self.worldview_queue.put(worldview_with_id)
        print(12)
            #worldview_queue.task_done()
        #else:
            #try:
                #print(14)
                #self.worldview_queue.get(True, 3) ##Removing essential information?
                #print(15)
                #self.worldview_queue.put(worldview_with_id)
                #print(16)
            #except:
                #print("Woops")

    def delete_lost_peers(self): ##FIX this
    	for id in self.worldview['elevators']:
    		if id in self.lost_peers:
    			del self.worldview['elevators'][id]
    			break

    #def elevator_is_ooo(self): #should be inside elevator
        #return 1

    def assign_orders(self):
        #if len(self.peers) >= 2:
        assigner_object = Assigner(self.worldview, self.id, self.peers) #local requests goes from 0 to 1 after assigner
        self.worldview = assigner_thingy.should_i_take_order()



    def sync_worldviews(self, worldview_foreign):
        id_foreign = next(iter(worldview_foreign))
        worldview_foreign = worldview_foreign[id_foreign]

        self.worldview['elevators'][id_foreign] = worldview_foreign['elevators'][id_foreign]
    	hall_orders = self.worldview['hall_orders']
    	hall_orders_foreign = worldview_foreign['hall_orders']
    	for floor in range (0, N_FLOORS):
    		for button in range (0, N_BUTTONS-1):
    				#if hall_orders[floor][button][0] != hall_orders_foreign[floor][button][0]:
    			if hall_orders[floor][button][1] < hall_orders_foreign[floor][button][1]:
    				hall_orders[floor][button][0] = hall_orders_foreign[floor][button][0]
    				hall_orders[floor][button][1] = hall_orders_foreign[floor][button][1]
    			else:
    				pass

    	self.worldview['hall_orders'] = hall_orders

    def update_order(self, order):
        floor, button, button_status = order
        self.worldview['hall_orders'][floor][button] = [button_status, time()]
        if button_status == 0:
            self.worldview['elevators'][self.id]['requests'][floor][button] = 0

    def get_id(self):
        return self.id

    def config_init(self):
        parser = ap.ArgumentParser(description='Port for simulation')
    	parser.add_argument('-p', '--port', dest='sim_port', required=False, metavar='<port_number>')
    	parser.add_argument('-i', '--id', dest='id', required=True, metavar='<elev_id')
    	args = parser.parse_args()
    	port_number = args.sim_port #Sender's email address
    	self.id = args.id
    	if port_number:
    		with open('C_interface/elevator_hardware.con', 'r') as file:
    			print(port_number)
    			data = file.readlines()

    		data[3] = "--com_port              " + port_number

    		with open('C_interface/elevator_hardware.con', 'w') as file:
    		    file.writelines(data)

    	else:
    		with open('C_interface/elevator_hardware.con', 'r') as file:
    			print(port_number)
    			data = file.readlines()

    		data[3] = "--com_port              " + '15657'

    		with open('C_interface/elevator_hardware.con', 'w') as file:
    		    file.writelines(data)
