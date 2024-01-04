'''
Version: 1
By: Tedi Qafko
Date: 12/19/2023

UHD Application with UI designed for inputing and outputing waveforms 
of types needs for SDRs

'''
from shell_scripts import *
from buttons import *

def setup_win(self, master, title):
    self.master = master
    self.screen_width = master.winfo_screenwidth() 
    self.screen_height = master.winfo_screenheight()
    self.w = w
    self.h = h
    self.x = (self.screen_width/2) - (self.w/2) # start x
    self.y = (self.screen_height/2) - (self.h/2) # start y
    master.geometry('%dx%d+%d+%d' % (self.w, self.h, self.x, self.y))
    master.title(title)

class USRP:
    def __init__(self, master):
        setup_win(self, master, 'USRP Setup')
        self.canvas = tk.Canvas(self.master)
        self.canvas.place(relx = 0.35, rely = 0.1)
        text_box = tk.Text(self.canvas, height = 20, width = 45) 
        text_box.grid(column = 0, row = 3)
        self.button1 = tk.Button(self.canvas, 
                                 text = 'Manual start UHD', 
                                 width = 55, 
                                 command = self.new_window)
        self.button1.grid(column = 0, row = 0)
        self.button2 = tk.Button(self.canvas, 
                                 text = 'UHD Find Devices', 
                                 width = 55, 
                                 command = lambda: uhd_find_devices(text_box))
        self.button2.grid(column = 0, row = 1)


    def new_window(self):
        self.newWindow = tk.Toplevel(self.master)
        tk.PanedWindow
        self.app = UHD(self.newWindow)

class UHD:
    def __init__(self, master):
        setup_win(self, master, 'UHD Waveform Generator')

        self.tabControl = ttk.Notebook(master) 
        self.tab1 = ttk.Frame(self.tabControl) 
        self.tab2 = ttk.Frame(self.tabControl) 
        self.tab3 = ttk.Frame(self.tabControl) 
        self.tabControl.add(self.tab1, text ='Tx_waveforms') 
        self.tabControl.add(self.tab2, text ='Tx_samples_from_file')
        self.tabControl.add(self.tab3, text ='Multi_samples_from_file')  
        self.tabControl.pack(expand = 1, fill ="both") 
        # ---------------------------- Tab 1 --------------------------
        tk.Label(self.tab1, 
                 text = "Welcome to tx_waveforms").grid(column = 0, 
                                                        row = 0, 
                                                        columnspan = 4,
                                                        padx = 15, 
                                                        pady = 15,
                                                        sticky = W)   
        inputs1 = add_waveform_params(self.tab1, 1, 1)
        t1 = terminal(self.tab1, 3, 2)
        start_btn(self.tab1, 3, 10, t1, inputs1)
        stop_btn(self.tab1, 3, 11, t1)
        # ---------------------------- Tab 2 --------------------------
        tk.Label(self.tab2, 
                 text = "Welcome to tx_samples_from_file").grid(column = 0, 
                                                                row = 0, 
                                                                columnspan = 4, 
                                                                padx = 15, 
                                                                pady = 15,
                                                                sticky = W) 
        inputs2 = add_waveform_params(self.tab2, 1, 1)
        inputs2.append(add_file_sample_params(self.tab2, 3, 1))
        t2 = terminal(self.tab2, 3, 2)
        start_btn(self.tab2, 3, 10, t2, inputs2)
        stop_btn(self.tab2, 3, 11, t2)
        # ---------------------------- Tab 3 --------------------------
        tk.Label(self.tab3, 
                 text = "Welcome to multi_samples_from_file").grid(column = 0, 
                                                                row = 0, 
                                                                columnspan = 4, 
                                                                padx = 15, 
                                                                pady = 15,
                                                                sticky = W) 
        inputs3 = add_waveform_params(self.tab3, 1, 1)
        inputs3.append(add_file_sample_params(self.tab3, 3, 1))
        inputs3.append(add_file_sample_params(self.tab3, 4, 1))
        inputs3.append(add_file_sample_params(self.tab3, 5, 1))
        inputs3.append(add_file_sample_params(self.tab3, 6, 1))
        t3 = terminal(self.tab3, 6, 2)
        start_btn(self.tab3, 3, 10, t3, inputs3)
        stop_btn(self.tab3, 3, 11, t3)
        # ------------------------------------------------------------
