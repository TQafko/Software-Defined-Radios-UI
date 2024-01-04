'''
Version: 1
By: Tedi Qafko
Date: 12/19/2023

UHD Application with UI designed for inputing and outputing waveforms 
of types needs for SDRs

'''

from tkinter import *
from tkinter import ttk
from parameters import *
from shell_scripts import *
import tkinter.filedialog

def add_waveform_params(tab, row, col):
    l1 = Label(tab, text = "Frequency:")
    l1.grid(row = row, column = col, sticky = E, padx = 0)

    col = col + 1
    e1 = Entry(tab, width = 15, bg = "white")
    e1.grid(row = row, column = col, sticky = W)

    col = col + 1
    l2 = Label(tab, text = "Rate:")
    l2.grid(row = row, column = col, sticky = E, padx = 0)

    col = col + 1
    e2 = Entry(tab, width = 15, bg = "white")
    e2.grid(row = row, column = col, sticky = W)

    col = col + 1
    l3 = Label(tab, text = "Gain:")
    l3.grid(row = row, column = col, sticky = E, padx = 0)

    col = col + 1
    e3 = Entry(tab, width = 15, bg = "white")
    e3.grid(row = row, column = col, sticky = W)

    col = col + 1
    l3 = Label(tab, text = "Channel:")
    l3.grid(row = row, column = col, sticky = E, padx = 0)

    col = col + 1
    e4 = Entry(tab, width = 15, bg = "white")
    e4.grid(row = row, column = col, sticky = W)
    
    return [e1, e2, e3, e4]

def add_file_sample_params(tab, row, col):
    l3 = Label(tab, text = "File sample:")
    l3.grid(row = row, column = col, columnspan = 2, sticky = E, padx = 0)
    col = col + 2
    e1 = Entry(tab, bg= "white")
    e1.grid(row = row, column = col, columnspan = 5, sticky = W + E)
    col = col + 5
    b1 = ttk.Button(tab, text="Browse File",
                    width = widthR1, 
                    command = lambda: output(tab, e1))
    b1.grid(row = row, column = col, sticky = W, columnspan = 2)
    return e1

def start_btn(tab, row, col, t, inputs):
    b1 = ttk.Button(tab, text="Run", width = widthR1, 
                    command = lambda: do_when_run(t, tab, inputs))
    b1.grid(row = row, column = col, pady = 5)

def do_when_run(t, tab, i):
    ''' Sends parameters to run a waveform generation command via UHD
    It determines the command based off the number of parameters
    in the inputs provided from the specific tabs
    Args: 
        t = terminal text box, 
        tab = tab of command
        i = text box with parameters freq, rate, gain, channel, files
    '''
    # TODO: Apply to general case of running any set of parameters
    if(len(i) == 4):
        # For tx_waveforms
        run(t, "192.168.10.4", str(i[0].get()), str(i[1].get()),
                               str(i[2].get()), str(i[3].get()))
    elif(len(i) == 5):
        # For playing a sample data
        run(t, "192.168.10.4", str(i[0].get()), str(i[1].get()),
                               str(i[2].get()), str(i[3].get()),
                               str("\"" + i[4].get() + "\""))
    elif(len(i) > 5):
        run(t, "192.168.10.4", str(i[0].get()), str(i[1].get()),
                               str(i[2].get()), str(i[3].get()),
                               None, # for file parameter skip
                               str(i[4].get()), str(i[5].get()), 
                               str(i[6].get()), str(i[7].get()))


def stop_btn(tab, row, col, t):
        b1 = ttk.Button(tab, text="Stop", width = widthR1,
                        command = lambda: stop(t))
        b1.grid(row = row, column = col, pady = 5)


def terminal(tab, row, col):
    # TODO: Parameters for text_box have to be put to parameters.py
    text_box = tk.Text(tab, height = height_textbox, width = 80) 
    text_box.grid(column = col, row = row + 5, 
                  columnspan = 8, 
                  sticky = S)
    # TODO: Fix pack scroll
    # scroll = tk.Scrollbar(tab) 
    # text_box.configure(yscrollcommand = scroll.set) 
    # scroll.config(command = text_box.yview) 
    # scroll.pack(side = tk.RIGHT, fill = tk.Y)
    return text_box


def output(tab, entry):
    path = tkinter.filedialog.askopenfilename(parent = tab)
    entry.delete(1, tkinter.END)  # Removes existing text in entry
    entry.insert(0, path)  # Insert the file path'
