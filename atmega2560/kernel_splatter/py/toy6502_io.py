#!/usr/bin/env python
# -*- coding: utf-8 -*-

###############################################################

# send a binary block over to a ram chip over uart 



import os
import sys

import serial
import time


# # # # #

#python2 use this 
#import Image 
#python3 use this  
#from PIL import Image 

# # # # #

# Mac OS: type 'ls /dev/tty.*' to see the serial ports,
SERIAL_PORT = '' #'/dev/tty.usbserial-FTWWRSIBA'  # '/dev/tty.usbserial-A400gnmx'  # 
OUTFILE_DIRECTORY = '/keith' # '/Users/klegg' # 
CONFIG_FILE = 'pyserial.cfg'  


RX_BUFFER = []
TX_BUFFER = []
FILEBUFFER = []


class device(object):

    def __init__(self, dryrun=False):
        self.PYSERIAL       = None  #python pyserial 
        self.SERIAL_PORT    = None  #'/dev/tty.usbserial' 
        self.TERMINATOR     = 0xb #*** suspect this does not matter as long as its not a number***
 
    
        self.filename       = 'image'
        self.bfilename      = 'dump.bin'
        self.tfilename      = 'dump.txt'

        self.load_config()

    def init(self):
        """ device constructor 
            need to add feature that detects if serial port is offline or has changed         
            automatically connects to serial port - you can bypass
            by sending "True" when you instantiate this class 
        """
        self.PYSERIAL = serial.Serial(port=self.SERIAL_PORT,
                                           baudrate=115200
                                           #timeout=1,  @with no timeout will block until it hits
                                           #???below???
                                           #dsrdtr=False,    #dsrdtr - Enable hardware (DSR/DTR) flow control.
                                           #xonxoff = False  #Enable - software flow control.
                                           #rtscts          #Enable  - hardware (RTS/CTS) flow control.
                                           ) 
        time.sleep(.5)
        self.PYSERIAL.flush()
        self.PYSERIAL.flushInput()
        #self.PYSERIAL.setDTR(False)

        #    self.g_comPort = serial.Serial(
        #        port = self.g_comPort_name,
        #        baudrate = 9600,
        #        parity = serial.PARITY_NONE,
        #        stopbits = serial.STOPBITS_ONE,
        #        bytesize = serial.EIGHTBITS,
        #        timeout = 10,
        #        dsrdtr = False,
        #        xonxoff = False,
        #        rtscts = False)

    ##----------------------##   
    def scribe(self, instr):
        print(instr)
    
    ##----------------------##  
    def binstr_to_hex(self, binstr):
        #basetwo = int(binstr, 2)
        return hex(int('binstr', 2))
 
    ##----------------------##   
    def freerun(self):
        """ launch 6502 into a freerun test """
        self.tx_char("z") #flood the ram with NOPS
        self.tx_char("c") #start up machine 

    ##----------------------##
    @property
    def get_in_buffer(self):
        siz_inbuf =  self.PYSERIAL.inWaiting() 
        return  self.PYSERIAL.read( siz_inbuf ) 

    ##----------------------##   
    def raw_machinecode(self, file, dryrun=False):
        """ 6502 machine code , poked into memory byte at a time 
            load a textfile of addresses and values and poke it to device
        """
        #check that file exists
        if not os.path.isfile(file): 
            sys.exit('\n\n Error opening %s/%s \n\n'%(os.getcwd(),file) )
             
        else:  
            cnt = 0  
            f = open(file, 'r')
            for line in f:
               toked = line.split(' ')  

               if len(toked)>=2:
                   addr = toked[0]
                   data = toked[1]
                   if '#' not in addr and '#' not in data:
                       if addr:
                           if dryrun:
                               print(addr, data) 
                           else:    
                               self.poke(int(addr), int(data) )
                           cnt+=1
            if not dryrun:
                print('transmitted %s bytes '%cnt )

    ##----------------------##

    ##----------------------##
    def pre_boot(self):
        print "\n\nTransmitting data over uart."

    ##----------------------##

    def post_boot(self):
        print "done. Starting up 6502 CPU."

    ##----------------------##
    def set_device(self, devtouse):
        self.SERIAL_PORT = devtouse 
    
    ##----------------------##        
    def load_config(self):
        """ load a config file - mostly for setting the 
            serial port properties 
        """ 
        if not os.path.isfile(CONFIG_FILE): 
            sys.exit('\n\n Error opening %s/%s \n\n'%(os.getcwd(),CONFIG_FILE) )
             
        else:    
            f = open(CONFIG_FILE, 'r')
            for line in f:
               toked = line.split(' ')          
               if toked[0]=='serial_port':
                   self.set_device(toked[1].replace('\n',''))
                   print '#serial port was set to %s'%toked[1] 
                   self.init()
     
    ##----------------------##
    def clear_buffers(self):
        TX_BUFFER     = []
        RX_BUFFER     = [] # raw serial buffer for one row (null terminated)
        FILEBUFFER    = []

        self.PYSERIAL.flushInput()
        self.PYSERIAL.flush()

    ##----------------------##
    def size_in_waiting(self):
        siz_inbuf =  self.TOY_IO.PYSERIAL.inWaiting() 
        #print ( self.TOY_IO.PYSERIAL.read( siz_inbuf ) ) 
        return siz_inbuf

    ##----------------------##
    def flush(self):
        self.PYSERIAL.flush()
        self.PYSERIAL.flushInput()

    ##----------------------##
    
    def test_for_echo(self, char):
        self.PYSERIAL.write( "q"  )
        self.PYSERIAL.write( char )
        #self.scribe( self.read_stream_in() )     #read until done (\0) 
        #self.scribe( self.read_fixed_buffer(12) )  #read set number  
        self.scribe( self.get_in_buffer )  #read set number  


    ##----------------------##
    def load_binary_text(self, bintextfile):
        """ load a binary disk file into (python) memory to be sent to device """

        if not os.path.isfile(bintextfile): 
            sys.exit('\n\n Error opening %s/%s \n\n'%(os.getcwd(),bintextfile) )
             
        else:    
            f = open(bintextfile, 'r')
            for l in f:
                self.FILEDATA.append(l)
    
    ##----------------------##
    def show_bin_numeric(self): 
        for b in self.DATA:
            print int(b, 2)
    ##----------------------##
    def rx_char(self, numbytes=1):
        return self.PYSERIAL.read(numbytes)    
    ##----------------------##
    def tx_char(self, char):
        """ send one byte to SEM 
            need to add feature that detects if serial port is offline or has changed 
        """
        #time.sleep(.01) #this solves some problems and creates others...
        if type(char)==int:
            char=chr(char)
        self.PYSERIAL.write( char )

    ##----------------------#https://accounts.google.com/ServiceLogin?service=mail&passive=true&rm=false&continue=https://mail.google.com/mail/&ss=1&scc=1&ltmpl=default&ltmplcache=2&emr=1&osid=1#
    #@property
    #def get_out_buffer(self):
    #    siz_inbuf =  self.PYSERIAL.inWaiting() 
    #    return  self.PYSERIAL.read( siz_inbuf )

    ##----------------------##
    def read_fixed_buffer(self, bufsize):
        return self.PYSERIAL.read(bufsize)        
    ##----------------------##
    def read_stream_in(self):
        """ readline seems bonky,numeric 10 character 
            can be mistaken for newline 
        """
        bufr = self.PYSERIAL.readline()
        bufr = bufr[:-1] #remove the newline
        return bufr
             
    ##---------------------
    def load_ram_image(self, binfile):
        """
           SEND TO 6502 - take a hex binary of 6502 machine langauge and send to hardware from terminal 
           CMD_RX_SRAM 
        """

        if os.path.lexists(binfile)==0:
            print('file "%s" does not exist'%binfile) 
            return None

        self.pre_boot()
        self.tx_char("u")
        f = open(binfile, "rb")
        try:
            byte = 1#f.read(1)
            while byte:
                byte = f.read(1)
                self.tx_char( byte )
        finally:
            f.close()
        self.post_boot()
    ##----------------------##
    def save_ram_image(self, filename):
        """
           RECEIVE FROM 6502 - perform a dump of SRAM to a terminal  
           CMD_TX_SRAM 
        """

        numbytes = 65535
        self.clear_buffers()
        self.tx_char("d")
        #time.sleep(15)

        DATA = self.read_fixed_buffer(numbytes)
        #self.DATA = self.PYSERIAL.read(numbytes) 
        

        #DATA = self.get_in_buffer  #4096 is BUFFER SIZE???
        
        #for byte in DATA:
        #    print( ord(byte))


        newFile = open (filename, "wb")
        self.scribe( 'writing file %s\n'%filename )
        cnt = 0
        for byte in DATA:
            #self.scribe( ord(byte) ) 
            #newFile.write( "%s\n" % byte )  #byte as text 
            newFile.write( byte )            #binary      
            cnt+=1

        newFile.close()
        os.system('chmod ugo+rwx %s'%filename)
   
        #with open(filename, 'wb') as f:
        #    f.write( ord(byte) )
        print "saved %s bytes"%cnt
        print "saved file "+filename
        #debug!
        #print self.DATA

    ##----------------------##
    def send_16bit(self, num):
        #split 16 into two bytes to be sent over serial 
        msb = (num >> 8) & 0xff
        lsb = num & 0xff
        self.tx_char(chr(lsb)) #lsb
        self.tx_char(chr(msb)) #msb

    ##----------------------##
    def peek(self, address):
        """ TODO - add a HEX display mode 
            serial byte representation to numeric hex
            
            CMD_PEEK_6502

        """
        self.tx_char("p")  
        self.send_16bit(address)
        print self.read_fixed_buffer(31)
        #self.scribe( self.get_in_buffer ) 

    ##----------------------##
    def set_px_region(self, byte1, byte2):
        self.tx_char("T")  
        self.tx_char(chr(byte1)) #lsb (treated as two though)
        self.tx_char(chr(byte2)) #msb (treated as two though)

    ##----------------------##
    def poke(self, address, data):
        """ CMD_POKE_6502
        """

        time.sleep(.1) #this is a bad hack - it is needed for repetitive calls to poke
        self.tx_char("o")
 
        if data<0 or data > 255:
            print 'data must be 8 bit unsigned'
            return None 

        if address>0 and address<256:
            self.tx_char(chr( address & 0xff))  #addr lsb
            self.tx_char(chr(0))                #addr msb        
            self.tx_char(chr(data))             #data value to poke

        if address>=256 and address<65535:
            msb = (address >> 8) & 0xff
            lsb = address & 0xff
            self.tx_char(chr(lsb))  #addr lsb
            self.tx_char(chr(msb))  #addr msb        
            self.tx_char(chr(data)) #data value to poke

        #print self.read_fixed_buffer(16+8)

    ##----------------------##
    def set_range(self, strt_addr, end_addr):
        """ CMD_SET_INTERNALS
        """
        self.tx_char("S")  
        self.send_16bit(strt_addr)
        self.send_16bit(end_addr)
        
        if strt_addr>end_addr:
            print "bad start addr "
            return None 

    ##----------------------##
    def peek_range(self, strt_addr, end_addr):
        """ CMD_PEEKRANGE
        """

        self.tx_char("P")  
        self.send_16bit(strt_addr)
        self.send_16bit(end_addr)
        
        if strt_addr>end_addr:
            print "bad start addr "
            return None 
         
        num_addrs = end_addr - strt_addr
        print self.read_fixed_buffer( num_addrs *31) #matches length of firmware print 


    ##----------------------##

    #CMD_SHOWINTERNALS
    def show_internals(self):
        """ CMD_SET_INTERNALS
        """
        self.flush()
        self.tx_char("s")
        time.sleep(.5)
        siz_inbuf =  self.PYSERIAL.inWaiting() 
        internals =  ( self.PYSERIAL.read( siz_inbuf ) ) 
        
        #TO SEE IN HEX - DEBUG NOT WORKING YET
        #addr_data = internals.split(' ')
        #for b in addr_data:
        #    print ( self.binstr_to_hex(b) )
        #    print (' ')

        print(internals)

    ##----------------------##
    
    # def set_internals(self, buf1, buf2):
    #     if buf1>65535 or buf2> 65535:
    #         print 'please limit to a 16 bit value' 
    #         return None
    #     self.tx_char("S")
    #     self.send_16bit(64000)
    #     self.send_16bit(64100)

    ##----------------------##
    def bscan_addr(self, lsb, msb):
        """ CMD_SET_ADDR_BUS 

        """ 
        self.tx_char("a") # set addr bus from next two bytes
        self.tx_char(lsb) #lsb
        self.tx_char(msb) #msb
        print self.read_fixed_buffer(30)


################################################################
################################################################

"""
   when gtkterm is running serial port can behave differently!
  it resets the device to run this if gtkterm is closed. WTF?
"""

###########################################################
"""
    def test_uart_tx_rx8(self):
        # debugging tool
        # read 8bit (0-255) interger tansmition from SEM 
  
        self.clear_buffers()
        self.PYSERIAL.write( "h"  )
        buffr_data = self.read_fixed_buffer(256)
        for c in buffr_data:
            self.scribe( ('#-> ',c, ord(c) ) )  

    ##----------------------##
    def test_uart_tx_rx10(self):
        # debugging tool
        #read 10bit (0-1024) interger tansmition from SEM 
 
        self.clear_buffers()
        self.PYSERIAL.write( "i"  )
        
        #buffr_data = self.read_stream_in()
        buffr_data = self.read_fixed_buffer(256)
        for c in buffr_data:
            self.scribe( ('#-> ',c, ord(c) ) )  



    def dump_buffer_text(self, flename=('/pixeldump.txt'), data=[]):
        print('writing file %s'%filename)
        newFile = open (filename, "w")
        for r in data:
            newFile.write( "%s %s" % (r, '\n') )  
        newFile.close()


"""
##----------------------##
"""
    def full_scan(self, testmode=False):
        self.autoset_res()
        self.clear_buffers()
        #this is the whole shebang 
        if not testmode:
            self.PYSERIAL.write("s")
        #testmode returns the same amount of data, an index of the loop  
        if testmode:
            self.PYSERIAL.write("j")            
        for r in range(0,self.longedge):
            RX_BUFFER = self.PYSERIAL.readline()
            RX_BUFFER = RX_BUFFER[:-1] #remove the newline
            self.IMAGE_BUFFER.append(RX_BUFFER) #add row to image
        self.scribe( '#received '+str(len(self.IMAGE_BUFFER))+' rows of pixels \n' )
    ##----------------------##
    def process(self, dodump=None):
        lowbyte  = 0 #lsb
        highbyte = 0 #msb
        voltage  = 0 #10 bit value (two bytes assembled from uart )
        ##
        dumpbuffer = []
        dumpcount = 0
        rowcount = 0 

        for row in self.IMAGE_BUFFER:
            row_vals = []
            count=0   
            rowcount=rowcount+1 
            for pix in row:
                if count==0:
                    lowbyte =( ord(pix) )
                if count%2==0:
                    lowbyte =( ord(pix) )
                #only capture on ODD bit so we have low and high 
                if count%2:
                    highbyte =( ord(pix) ) 
                    if lowbyte>=256:
                        highbyte=highbyte-1      
                    voltage = (highbyte*256)+lowbyte 
                    row_vals.append(voltage)  
                    # # # # # 
                    if dodump:
                        dumpbuffer.append( "count:%s  rawpix %s high %s low %s- output %s" % 
                            (str(count).zfill(4), str(ord(pix)).zfill(4), str(highbyte).zfill(4), str(lowbyte).zfill(4), voltage) )
           
                count=count+1
            if dodump:
                dumpbuffer.append('\n') 

            #print('process : row length ', row_vals[1], len(row_vals) )
            self.pix_vals.append(row_vals) #sorted buffer 
        
        #################################
        #raw dump of high and low bytes sent over for debugging
        if dodump:
            self.dump_buffer_text( (OUTFILE_DIRECTORY+'/pixeldumpraw.txt'),
                                    dumpbuffer)
            self.scribe( ('converted ', len(self.pix_vals) , 'rows , row size: ', len(self.pix_vals[1])) )
    
"""


    ##----------------------##
    ##----------------------##
    #def receive_sample_data_2d(self):
    #    """ debugging tool 
    #        send a 2D sequence of bytes 
    #        from the hardware and write them to a file client side 
    #    """
    #    self.clear_buffers()
    #    self.PYSERIAL.write("t")
    #    for r in range(0,self.longedge):
    #        RX_BUFFER =[]
    #        RX_BUFFER = self.PYSERIAL.readline()
    #        RX_BUFFER = RX_BUFFER[:-1] #remove the newline
    #        self.IMAGE_BUFFER.append(RX_BUFFER) #add row to image
    #    self.scribe( '#received %s rows of pixels \n'%str(len(self.IMAGE_BUFFER)) )

    ##----------------------##
    #def generate_sample_data(self):
    #    """ debugging tool 
    #        play with pixel math , etc 
    #        I think it uses an as yet unbuilt testing tool on SEM 
    #        DEBUG UNFINISHED  
    #    """
    #    self.clear_buffers()
    #    imagesize = 256
    #    for rx in range(0,self.longedge):
    #        #self.IMAGE_BUFFER.append(r) 
    #        thisrow = [] 
    #        for ry in range(0,self.longedge): 
    #            thisrow.append(rx)
    #        self.pix_vals.append(thisrow)


    ##----------------------##
    # def receive_terminated_stream(self):
    #     """ debugging tool """
    #     self.clear_buffers()
    #     self.PYSERIAL.write("t")
    #     self.IMAGE_BUFFER = self.PYSERIAL.readline()
    #     self.scribe( '#received %s bytes \n'%str(len(self.IMAGE_BUFFER)) )
    # ##----------------------##
    # def receive_stream_data(self):
    #     """ debugging tool """
    #     self.clear_buffers()
    #     self.PYSERIAL.write("t")
    #     for b in range(256):
    #          self.IMAGE_BUFFER.append(self.PYSERIAL.read() )
    #     self.scribe( '#received %s bytes \n'%str(len(self.IMAGE_BUFFER)) )
