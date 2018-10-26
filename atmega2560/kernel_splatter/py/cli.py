
import os
import sys
import toy6502_io

import time

"""
   TOY COMPUTER 
"""



##
class CLI(object):

    def __init__(self):
        self.TOY_IO      = toy6502_io.device()
        self.command     = ''
        self.split       = [] 
        self.lastcommand = ""
        ## ## ## 
        self.HISTORY     = [] #command history

    def cpr(self, strdat):
        print( str(strdat)+'\n' )

    def cpr_ns(self, strdat):
        print( strdat )  
    ##
    def toy_console(self):  

        while self.command !='quit':
            if self.command != 'help' :
                self.cpr_ns('\n\n')
            
            self.cpr('## ## ## ## ## ENTER A COMMAND ## ## ## ## ## ##')
            self.lastcommand = self.command
            
            #python2
            if sys.version_info[0] < 3:
                self.command = str(raw_input() )
            #python3
            else:
                self.command = str(input() )

            self.split   = self.command.split(" ")

            ###########
            if self.command == 'exit' or self.command == 'x': 
                break
            if self.split[0] == 'exit' or self.split[0] == 'x': 
                break

            if self.command == 'about' or self.split[0] == 'about': 
                self.cpr_ns( '--> Keith Legg, Perihelionvfx  2015-20XX <--')
            
            if self.split[0] == 'history' or self.command == 'history': 
                self.cpr_ns( self.HISTORY )
            else:
                self.HISTORY.append( str(self.command) ) 

            ########
            if self.command == 'help': 
                self.cpr_ns('## ## ## ## ##       6502 HELP       ## ## ## ## ## ##  ')   
                self.cpr_ns('  inwaiting, iw           - flush buffer and print conetents   ')
                self.cpr_ns('  poke                    - set a memory value                 ')
                self.cpr_ns('  peek                    - get a memory value                 ')
                self.cpr_ns('  setrange (start, end)   - set range globals on 6502          ')
                self.cpr_ns('  peekrange               - get range globals on 6502         ')
                self.cpr_ns('  load (file)             - load a binary file to 6502        ')
                self.cpr_ns('  save (file)             - save a binary file to 6502        ') 
                self.cpr_ns('  send/set                - send a byte over serial port      ')  
                self.cpr_ns('  get                     - // get a byte from serial port    ') 
                self.cpr_ns('  echo                    - test serial port                  ')    
                self.cpr_ns('  raw/proof (file)        - poke machine code into SRAM        ')  
                self.cpr_ns('  clear                   - set all SRAM to zeroes             ')    
                self.cpr_ns('  ls                      - list all files in directory        ')  
                self.cpr_ns('  setaddr                 - boundary scan byte to address     ') 
                #self.cpr_ns('  setaddr                - boundary scan byte to address     ') 

                self.cpr_ns('  ---------------------------------------------  ')                

                self.cpr('\n\n')


            ###########             ###########            ###########
            ###########             ###########            ########### 
            ###########             ###########            ###########
            
            ###########
            if self.split[0] == 'set_device' : 
                self.cpr_ns( 'changing device to %s'%self.split[1] )
                self.TOY_IO.set_device( self.split[1] )

            ########### 
            # DEBUG UNFINSHED        
            if self.command == 'show': 
                #self.TOY_IO.flush()
                self.TOY_IO.show_internals()

            ########### 
            # DEBUG UNFINSHED        
            if self.command == 'inwaiting' or self.command == 'iw': 
                print ( '----------------------------------------' )                 
                print( self.TOY_IO.get_in_buffer  )


            ###########   
            if self.command == 'clear': 
                self.TOY_IO.tx_char('x')


            ########### 
            if self.command == 'draw': 
                self.TOY_IO.tx_char('N') #clear screen
                self.TOY_IO.tx_char('B') #draw from SRAM

            ########### 
            if self.split[0] == 'px': 
                self.TOY_IO.set_px_region(int(self.split[1]), int(self.split[2]) )


            ########### 
            if self.command == 'cls': 
                self.TOY_IO.tx_char('N')

            ########### 
            if self.command == 'nes': 
                self.TOY_IO.tx_char('n')

            ###########   
            if self.command == 'ls': 
                for f in ( os.listdir(os.getcwd() ) ):
                    print(' ->',f)


            ###########   
      
            if self.command == 'raw': 
                print ('raw needs args')
            if self.split[0] == 'raw' :
                print('loading raw file %s to 6502.'%self.split[1])
                self.TOY_IO.raw_machinecode(self.split[1])

            #same as raw but in dryrun mode   
            if self.command == 'proof': 
                print ('proof needs args')
            if self.split[0] == 'proof' :
                print('loading raw file %s to 6502.'%self.split[1])
                self.TOY_IO.raw_machinecode(self.split[1], True)

            ###########                             
            # DEBUG UNFINSHED        
            if self.command == 'load': 
                print ('load needs args')
            if self.split[0] == 'load' or self.split[0] == 'load':
                print('transmitting %s to 6502.'%self.split[1])
                self.TOY_IO.load_ram_image(self.split[1])
             
                #print( self.TOY_IO.get_in_buffer )


            ###########

            # DEBUG UNFINSHED        
            if self.command == 'save': 
                print ('save needs args')
            if self.split[0] == 'save' or self.split[0] == 'save':
                #  siz_inbuf =  self.TOY_IO.PYSERIAL.inWaiting() 
                #   print ( self.TOY_IO.PYSERIAL.read( siz_inbuf ) ) 
                #  save_ram_image
                
                self.TOY_IO.save_ram_image(self.split[1])

                pass#self.TOY_IO
 
            ########### 
            if self.command == 'poke': 
                print ('needs 2 args (addr, byte)')
            if self.split[0] == 'poke':                                  
                print('poking %s %s '%(self.split[1], self.split[2])  )
                self.TOY_IO.poke( int(self.split[1]), int(self.split[2]) )

            ###########  

            if self.command == 'peek': 
                print ('# peeking at reset vector')
                self.TOY_IO.flush()
                self.TOY_IO.peek( 65532 )
                self.TOY_IO.peek( 65533 )
            elif self.split[0] == 'peek':  
                self.TOY_IO.flush()                              
                self.TOY_IO.peek( int( self.split[1]) )
                #time.sleep(1)
                #print( self.TOY_IO.get_in_buffer )

            ###########             
            if self.command == 'testbin' : 
                """ DEBUG UNFINSHED """
                self.TOY_IO.clear_buffers()
                self.TOY_IO.receive_stream_data()
                #self.SEM_IO.process() 
                self.TOY_IO.write_raw_binary()

            ###########
            if self.command == 'echo' :
                self.cpr('requires 1 argument')
            ## ## ## ## ##                
            if self.split[0] == 'echo' : 
                self.cpr_ns( 'echoing (sending) %s to device'%self.split[1] )
                self.TOY_IO.test_for_echo( self.split[1] )

            ###########                
            if self.split[0] == 'set' or self.split[0] == 'send': 
                self.cpr_ns( 'Sending %s to device.'%self.split[1] )
                self.TOY_IO.tx_char( self.split[1] )

            ###########                
            if self.split[0] == 'setdbus' : 
                #self.cpr_ns( 'Sending %s to device.'%self.split[1] )
                self.TOY_IO.tx_char( self.split[1] )

            ###########                
            if self.split[0] == 'get' : 
                self.cpr_ns( 'Listening over serial for %s bytes.'%self.split[1] )
                print(self.TOY_IO.rx_char( int(self.split[1]) ) )

            ###########                
            if self.split[0] == 'run' :
                """ quick method to start 6502 running to keep its chip toy_console
                    the thing gets really hot after startup if not running ???
                    . . .
                    not sure why yet. I think it happens when sram is zerod and 
                    control bus is set to HI-Z on AVR side to initialize 
                """
                self.cpr_ns( 'Starting up 6502 toy computer. Jolly Good.')
                self.TOY_IO.tx_char('z')
                time.sleep(1)
                self.TOY_IO.tx_char('c')

            
            ###########                
            if self.split[0] == 'setrange' or  self.split[0] == 'sr': 
                """  rx_four_bytes
                     CMD_SET_INTERNALS 

                """

                self.TOY_IO.clear_buffers()
                print('#setting range to %s-%s'%(self.split[1], self.split[2]) )
                self.TOY_IO.set_range( int(self.split[1]), int(self.split[2]) )

            ###########                
            if self.split[0] == 'peekrange' or self.split[0] == 'pr': 
                """  rx_four_bytes
                     CMD_SET_INTERNALS 
                """
                
                self.TOY_IO.clear_buffers()
                print('#setting range to %s-%s'%(self.split[1], self.split[2]) )
                self.TOY_IO.peek_range( int(self.split[1]), int(self.split[2]) )

                time.sleep(.3)
                print( self.TOY_IO.get_in_buffer  )

            ###########                
            if self.split[0] == 'flush' : 
                print( self.TOY_IO.get_in_buffer  )               
                self.TOY_IO.PYSERIAL.flush()

                

            ########### 

            #

            ########### 



####################
toy_cli = CLI()
toy_cli.toy_console()