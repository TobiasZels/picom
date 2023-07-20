import os
import keyboard

FIFO_PATH = "../studyfifo"

exitVariable = False
lastKey = '0'

def write_to_fifo(data):
    try:
        fifo = open(FIFO_PATH, "w")
        fifo.write(data)
        fifo.close()
        print("success")
    except FileNotFoundError:
        print("FIFO not found")

def send_data(data):
    hashmap = {"lastKey": data}
    hashmap_data = "\n".join([f"{key}:{value}" for key, value in hashmap.items()])

    write_to_fifo(hashmap_data)

def key_press_event(event):
    global lastKey
    if event.name == "6":
        exitVariable = True
        print(lastKey, event.name)
    if lastKey != event.name:
        print("sendingData")
        lastKey = event.name
        send_data(event.name)

    print(event.name)

keyboard.on_press(key_press_event)

while(not exitVariable):
    exitVariable = False
