'''
Version: 1
By: Tedi Qafko
Date: 12/19/2023

UHD Application with UI designed for inputing and outputing waveforms 
of types needs for SDRs

'''
from __future__ import annotations
import subprocess
import threading
import tkinter as tk
    
def run(text_box, ip, f, r, g, ch, file = None,
                                   file1 = None, file2 = None, 
                                   file3 = None, file4 = None):
    ip_arg = "addr=" + ip
    if(file != None and file1 == None and file2 == None and 
                        file3 == None and file4 == None):
        # Run tx_samples_from_file
        run_command(["/usr/lib/uhd/examples/tx_samples_from_file",
                 "--args", ip_arg,
                 "--freq", f,
                 "--rate", r,
                 "--gain", g,
                 "--channel", ch,
                 "--file", file],
                   text_box)
    elif(file1 != None and file2 != None and 
                           file3 != None and file4 != None):
        # Run multi_files_from_sample
        ip_arg = "addr1=" + ip + ",addr2=192.168.10.2"
        run_command(["/usr/lib/uhd/examples/multi_files_from_sample",
                    "--args", ip_arg,
                    "--freq", f,
                    "--rate", r,
                    "--gain", g,
                    "--channel", ch,
                    "--file1", file1,
                    "--file2", file2,
                    "--file3", file3,
                    "--file4", file4],
                    text_box)
    else:
        # Run tx_waveforms
        run_command(["/usr/lib/uhd/examples/tx_waveforms",
                 "--args", ip_arg,
                 "--freq", f,
                 "--rate", r,
                 "--gain", g,
                 "--channel", ch],
                   text_box)

def test(text_box):
    print("Running")
    # Test code
    # subprocess.run(["ls", "-l"])
    # text_box.insert(tk.END, "command started")
    # run_command(["ping"], text_box)
    run_command(["ping", "192.168.10.2"], text_box)

def stop(text_box):
    try:
        proc.kill()
        text_box.insert(tk.END, "Done\n")
        text_box.insert(tk.END)
        text_box.yview(tk.END)
    except Exception as error:
        text_box.insert(tk.END, "\nNo process is running\n")

def run_command(command: list[str], text_box: tk.Text):
    """Run a command and redirect output to a text box in real-time"""
    global proc 
    text_box.insert(tk.END, command)
    text_box.insert(tk.END, "\n")
    try:
        proc = subprocess.Popen(command,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.STDOUT,
                                shell = False)
        # Start a separate thread to update the text box with output
        thread = threading.Thread(target = update_text_box(text_box,
                                                           proc))
        thread.start()
    except Exception as error:
        text_box.insert(tk.END, "Error!\n")

def update_text_box(text_box: tk.Text, proc: subprocess.Popen):
    """ Create a closure to capture proc and 
    text_box which will be called to update the text box. """
    for line in proc.stdout:
        text_box.insert(tk.END, line.decode())
        text_box.yview(tk.END)

def uhd_find_devices(t):
    run_command(["uhd_find_devices"], t)