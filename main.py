'''
Version: 1
By: Tedi Qafko
Date: 12/19/2023

UHD Application with UI designed for inputing and outputing waveforms 
of types needs for SDRs

'''
from tkinter import *
from windows import *

def main(): 
    win = Tk()
    USRP(win)
    win.mainloop()

if __name__ == '__main__':
    main()