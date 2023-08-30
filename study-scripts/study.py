import os
from random import choice
import keyboard
import threading 
import csv
from datetime import datetime
import time
import argparse

FIFO_PATH = "../studyfifo"
argParser = argparse.ArgumentParser()
argParser.add_argument("-id", "--participantID", help="participant id")
args = argParser.parse_args()
id = args.participantID
exitVariable = False
lastKey = '0'
AVAILABLE_FRAMERATES = ['60','120', '144', '240']
AVAILABLE_MAKER = ['qr', 'aruco', 'point']
AVAILABLE_SCENARIOS = ['text_w', 'text_d', 'image']

SCENARIO_TIMER = 5

#variables
framerates = []
marker = []
scenarios = []
nextImage = False

def shuffle_study():
    temp_array = AVAILABLE_FRAMERATES.copy()
    for _ in AVAILABLE_FRAMERATES:
        entry = choice(temp_array)
        framerates.append(entry)
        temp_array.remove(entry)

    temp_array = AVAILABLE_MAKER.copy()
    for _ in AVAILABLE_MAKER:
        entry = choice(temp_array)
        marker.append(entry)
        temp_array.remove(entry)
    
    temp_array = AVAILABLE_SCENARIOS.copy()
    for _ in AVAILABLE_SCENARIOS:
        entry = choice(temp_array)
        scenarios.append(entry)
        temp_array.remove(entry)
    

def write_to_fifo(data):
    try:
        fifo = open(FIFO_PATH, "w")
        fifo.write(data)
        fifo.close()
        print("success")
    except FileNotFoundError:
        print("FIFO not found")

def send_data(marker, scenarios, framerate, time_out=False):
    hashmap = {"marker": marker, "scenarios": scenarios, "framerate": framerate, "timeout": time_out}
    hashmap_data = "\n".join([f"{key}:{value}" for key, value in hashmap.items()])

    write_to_fifo(hashmap_data)

def log(rating):
    global sc_value, fr_value, mk_valuecc
    print("sc " + str(sc_value))
    print("fr " + str(fr_value))
    print("mk " + str(mk_value))
    time_stamp = time.time()
    time_stamp = datetime.fromtimestamp(time_stamp)
    fieldnames = ['id', 'timestamp', 'task', 'framerate', 'marker', 'rating']
    row = {'id': id, 'timestamp': time_stamp, 'task': scenarios[sc_value], 'framerate': framerates[fr_value],'marker': marker[mk_value], 'rating': rating}
    with open('logfiles/log.csv', 'a', newline='') as f:
        dictwriter_object = csv.DictWriter(f, fieldnames=fieldnames)
        dictwriter_object.writerow(row)
        f.close()
    print(rating)

beginning = True

def key_press_event(event):
    global lastKey
    global nextImage
    global timeout
    if beginning:
        if event.name == "5":
            nextImage = True
            timeout = False
            beginning = False
            
    if timeout and not exitVariable and not beginning:
        match event.name:
            case "1":
                log(1)
                nextImage = True
                timeout = False
            case "2":
                log(2)
                nextImage = True
                timeout = False

            case "3":
                log(3)
                nextImage = True
                timeout = False
            
            case "4":
                log(4)
                nextImage = True
                timeout = False

            case "5":
                log(5)
                nextImage = True
                timeout = False

    print(event.name)

keyboard.on_press(key_press_event)
shuffle_study()
sc_value = 0
mk_value = -1
fr_value = 0
timeout = True
def time_out():
    global timeout
    if not nextImage:
        print("Timeout")
        timeout = True
        send_data(marker[mk_value], scenarios[sc_value], framerates[fr_value], True)

def startProgramm(screen):
    print(screen)
    #match screen:
    #    case ""

    # Set fullscreen
    #os.system("xdotool key super+2")

    
os.system('xrandr --output DP-4 --mode 1920x1080 --rate ' + framerates[0] )
iterations = 0
max_iterations = 3

while(not exitVariable):
    if(nextImage):
        print("nextScenario")
        nextImage = False
        if iterations < max_iterations:
            if fr_value < len(framerates)-1 or sc_value < len(scenarios) -1 or mk_value < len(marker) -1:
                if sc_value < len(scenarios)-1 or mk_value < len(marker) -1:
                    if mk_value < len(marker) -1:
                        mk_value += 1
                    else:
                        mk_value = 0
                        sc_value += 1
                else:
                    sc_value = 0
                    mk_value = 0
                    fr_value += 1
                    os.system('xrandr --output DP-4 --mode 1920x1080 --rate ' + framerates[fr_value] )
                    time.sleep(2)
            else:
                sc_value = 0
                mk_value = 0
                fr_value = 0
                framerates = []
                marker = []
                scenarios = []
                shuffle_study()

                os.system('xrandr --output DP-4 --mode 1920x1080 --rate ' + framerates[fr_value] )
                time.sleep(2)
                iterations += 1
        else:
            exitVariable = True

        timer = threading.Timer(SCENARIO_TIMER, time_out)
        timer.start()
        # Start the Program + emulate mod + f for fullscreen
        startProgramm(scenarios[sc_value])
        send_data(marker[mk_value], scenarios[sc_value], framerates[fr_value], False)
        if exitVariable:
            send_data(marker[mk_value], "fin", framerates[fr_value], False)



    