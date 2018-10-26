#!/usr/bin/env python
# -*- coding: utf-8 -*-



from toy6502_io import * 


##############################



"""
foo = device()
foo.asm_test()
time.sleep(1)
foo.peek_range(0, 100)
#time.sleep(1)
#foo.peekrange(65532, 65535)
"""


"""
foo.peekrange(65530, 65535)
foo.tx_char('s')
print foo.rx_char(33)
"""


###################################
###################################
###################################



###################################
# for a in range(50):
#     foo.poke(a, a)
#     time.sleep(.1)
# time.sleep(2)
# foo.peekrange(0,50)
###################################

###################################
# echo all 8 bit values
#for x in range(256):
#    foo.test_for_echo(chr(x) )