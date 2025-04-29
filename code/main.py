#!/opt/home-auto/venv3.11/bin/python

# General modules
import os
import sys
import atexit

# Dedicated modules
import subprocess

# Personal modules
sys.path.append("/opt/Python-Utils/utils/")
from utils import Logger,DataBase,Timer,GetFuncName,GetTime
from api import Api

# Init logger
_logger=Logger(log_path="/opt/home-auto/log/home.log")
# Unhandled exceptions
sys.excepthook = _logger.exception_handler

# Init timer
timer = Timer()

class Base:
	def __init__(self):
		self.unipi_sys_base_dir=""
		self.database_info={}
	def set_running_mode(self,virtual_mode:bool=False):
		# Start in normal or virtual mode
		VIRTUAL_MODE=virtual_mode
		if VIRTUAL_MODE:
			# Initialize virtual mode variables
			self.unipi_sys_base_dir="/opt/home-auto/code/virtual_pins/"
			self.database_info={
				"Host":"127.0.0.1"
				,"User":"joan"
				,"Password":"2126"
				,"DataBase":"home_automation"
			}
		else:
			# Initialize normal mode variables
			self.unipi_sys_base_dir="/run/unipi-plc/by-sys/"
			self.database_info={
				"Host":"192.168.1.100"
				,"User":"joan"
				,"Password":"2126"
				,"DataBase":"home_automation"
			}

class DigitalPin(Base):
	def __init__(self,aPin:str):
		super().__init__()
		self.set_running_mode(virtual_mode=True)
		self.iPin=aPin # 2.(pin)
	
	def read(self):
		status = subprocess.run(["cat", f"{self.unipi_sys_base_dir}DI{self.iPin}/value"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

		if status.returncode != 0:
			err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
			_logger.error(err)
			return err
		else:
			data={"id":self.iPin,"state":str(status.stdout.decode().strip())}
			return data

class Relay(Base):
	def __init__(self,aPin:str):
		super().__init__()
		self.set_running_mode(virtual_mode=True)
		self.iPin=aPin
		# Start the relay if specified in calendar
		self.calendar=Calendar(self)
		self.start_relay()

		# Init historify for a relay
		self.hist=Historify(self,aTable="historify")
	
	def read(self):
		status = subprocess.run(["cat", f"{self.unipi_sys_base_dir}RO{self.iPin}/value"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

		if status.returncode != 0:
			err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
			_logger.error(err)
			return err
		else:
			data={"id":self.iPin,"state":str(status.stdout.decode().strip())}
			return data
	
	def write(self,newState:int):
		"""
        Change a relay's state on or off

        newState: 1 (on), 0 (off)

        """

		" echo 1 | sudo tee value"
		status = subprocess.run(f"echo {str(newState)} | sudo tee {self.unipi_sys_base_dir}RO{self.iPin}/value", 
                        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

		if status.returncode != 0:
			err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
			_logger.error(err)
			return err
		else:
			data={"status":200}
			return data
	
	def start_relay(self):
		"""
		Start the relay if specified in calendar
		"""

		if not self.calendar.isActive or (self.calendar.startDate and self.calendar.endDate):
			self.write(0)
			return
		
		now=GetTime()
		if now >= self.calendar.startDate and now <= self.calendar.endDate:
			self.write(newState=1)
		elif now < self.calendar.startDate or now > self.calendar.endDate:
			self.write(newState=0)
		


class Calendar:
	def __init__(self,aInstance:Relay):
		self.relay=aInstance
		self.isActive=False
		self.startDate=None
		self.endDate=None
		self.calendarInfo:dict={}
		self.iDb=DataBase(
			Host=self.relay.database_info["Host"]
			,User=self.relay.database_info["User"]
			,Password=self.relay.database_info["Password"]
			,DataBase=self.relay.database_info["DataBase"]
		)
		self.get_data()
	def set_data(self):
		# {
		#     "calendar": {
		#         "id": "RO2.1",
		#         "start_date": "",
		#         "end_date": "",
		#         "date_active": 0
		#     }
		# }
		iData=self.calendarInfo["calendar"]
		iStartDate=iData["start_date"] if iData["start_date"]!="" else None
		iEndDate=iData["end_date"] if iData["end_date"]!="" else None
		iDateActive=iData["date_active"]
		self.startDate=iStartDate
		self.endDate=iEndDate
		self.isActive=[False,True][iDateActive==1]
	def get_data(self):
		try:
			"""
			Start the relay at a specific date
			"""
			self.iDb.connect()
			if self.iDb is None:
				err=f"Could not connect to bd in Relay.Calendar.{self.get_data.__name__} function"
				_logger.error(err)
				pass
			iSql=f"SELECT * FROM relay WHERE id='RO{str(self.relay.iPin)}'"
			self.iDb.execute(aQuery=iSql)
			iRet=self.iDb.fetchdata()
			self.iDb.close()
			if iRet == [] or not isinstance(iRet,list):
				err="Could not fetch data : "+str(self.get_data.__name__+" function")
				err+="\n"
				err+=str(self.iDb.fetchdata.__name__)+" function response: "+str(iRet)
				_logger.error(err)
				raise Exception(err)
			
			self.calendarInfo["calendar"]=iRet[0]
			self.set_data()
			
		except Exception as e:
			_logger.error(f"Error in {str(self.get_data.__name__)} : {str(sys.exc_info())} : {str(e)}")

class Historify:
	"""
	Historify changes in pin states
	"""
	def __init__(self,aInstance:Relay|DigitalPin,aTable:str):
		self.iTable=aTable
		self.idrelay=aInstance.iPin
		self.iDb=DataBase(Host="192.168.1.100",User="joan",Password="2126",DataBase="home_automation")
	def add(self,new_pin_state:str):
		try:
			self.iDb.connect()
			iTs=GetTime(aTimeZone="Europe/Madrid",accuracy="s")
			iParams=[str(self.idrelay),str(new_pin_state),str(iTs)]
			iSql="INSERT INTO `{}` (idrelay,newstate,ts) VALUES (%s,%s,%s)".format(str(self.iTable))
			self.iDb.execute(iSql,iParams)
			self.iDb.close()
			# Debug
			# iSql=f"INSERT INTO {iParams[0]} (idrelay,newstate,ts) VALUES ('{iParams[1]}','{iParams[2]}','{iParams[3]}')"
			# _logger.debug(iSql)
		except Exception as e:
			err="Error in Relay.Historify.add function : "+str(sys.exc_info())+" : "+str(e)
			_logger.error(err)

class RelayHandler:
	def __init__(self):
		self.relay1=Relay(aPin="2.1")
		self.relay2=Relay(aPin="2.2")
		self.relay3=Relay(aPin="2.3")
		self.relay4=Relay(aPin="2.4")
		self.relay5=Relay(aPin="2.5")
		self.relay6=Relay(aPin="2.6")
		self.relay7=Relay(aPin="2.7")
		self.relay8=Relay(aPin="2.8")
	
	def get_relay(self,aPin):
		match aPin:
			case "2.1":
				return self.relay1
			case "2.2":
				return self.relay2
			case "2.3":
				return self.relay3
			case "2.4":
				return self.relay4
			case "2.5":
				return self.relay5
			case "2.6":
				return self.relay6
			case "2.7":
				return self.relay7
			case "2.8":
				return self.relay8
		return None


def ReadPin(data,aPin:str):
	iPin = rl_handler.get_relay(aPin)
	return iPin.read()

def ReadAllPins():
    pins_state={
        "pins": []
    }
    for i in range(1,9):
        iPin="2."+str(i)
        pin=rl_handler.get_relay(iPin)
        pin_status = pin.read()
        pins_state["pins"].append(pin_status)
    return pins_state

def WriteRelay(data,aPin:str,aStatus:str):
	aStatus=int(aStatus)
	if aStatus != 0 and aStatus != 1:
		return {"error": f"Relay status {aStatus} can't be set: incorrect status form"}
	relay = rl_handler.get_relay(aPin)
	relay.hist.add(new_pin_state=aStatus)
	
	return relay.write(newState=aStatus)

def GetCalendar(data,aPin:str):
	relay=rl_handler.get_relay(aPin)
	calendar_data=relay.calendar.calendarInfo
	return calendar_data

if __name__ == "__main__":
	rl_handler=RelayHandler()
	_api = Api(Port=8000,logger=_logger,init_message="Started home automation api",exit_message="Closed home automation api",allowed_origins=["http://100.116.80.15:8020","http://homeserver:8020","http://100.117.134.68","http://127.0.0.1"])
	_api.add_get_request("/api/ReadAllPins",ReadAllPins)
	_api.add_post_request(r"/api/ReadPin/(?P<aPin>2\.[1-9])",ReadPin)
	_api.add_post_request(r"/api/WriteRelay/(?P<aPin>2\.[1-9])/(?P<aStatus>[0-1])",WriteRelay)
	_api.add_post_request(r"/api/GetCalendar/(?P<aPin>2\.[1-9])",GetCalendar)
	_api.init_app()

