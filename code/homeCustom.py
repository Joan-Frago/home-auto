import sys

from pyutils import Logger
from homeMain import Relay,DigitalPin

import threading

_logger=Logger(log_path="/opt/home-auto/log/home.log",enable_rotation=False,max_log_file_size=90)
sys.excepthook = _logger.exception_handler


class Persiana:
    def __init__(self, rl_instance:Relay, di_instance:DigitalPin):
        self.relay = rl_instance
        self.diInput = di_instance

    def run(self):
        try:
            iTarget=self.thread
            iThreadName="Thread_persiana"
            self.thread=threading.Thread(target=iTarget,name=iThreadName,daemon=True)
            self.thread.start()
            
        except Exception as e:
            err = str(e) + " : " + str(sys.exc_info())
            print(err)

    def thread(self):
        try:
            while True:
                if self.diInput.start_relay:
                    self.relay.write(newState=1)
                    
                    _logger.debug("Persiana encesa correctament")
                else:
                    self.relay.write(newState=0)
        except Exception as e:
            raise Exception(e)




