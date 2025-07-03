#!/opt/home-auto/venv3.11/bin/python

# General modules
import os
import sys
import atexit

# Dedicated modules
import subprocess
import threading

# Personal modules
from pyutils.utils import Logger,DataBase,Timer,GetFuncName,GetTime,get_json_data,dict2json,writeFile,wait,GetEpochTimestamp,TimestampTimeDiff

# Init logger
_logger=Logger(log_path="/opt/home-auto/log/home.log",enable_rotation=False,max_log_file_size=90)
# Unhandled exceptions
sys.excepthook = _logger.exception_handler

class Base:
    def __init__(self
                 ,aBaseDir:str
                 ,aDbInfo:dict
                 ,aPin:str
                 ,aName:str
                 ,aDesc:str
                 ,aIsVirtual:bool
                 ,aType:str
                 ,aIsHist:bool
                 ,aHistPeriod:int
                 ,aIO:str=None
                 ):
        self.unipi_sys_base_dir=aBaseDir
        self.database_info=aDbInfo
        self.iIdPin=aPin
        self.iName=aName
        self.iDesc=aDesc
        self.iIsVirtual=aIsVirtual
        self.iType=aType
        self.iIO=aIO
        self.iIsHist=aIsHist
        self.iHistPeriod=aHistPeriod

        self.run_mode=""
        self.application_running=True
        self.forced_state=False

        self.iDb=DataBase(
            Host=self.database_info["Host"]
            ,User=self.database_info["User"]
            ,Password=self.database_info["Password"]
            ,DataBase=self.database_info["DataBase"]
            ,buffered=True
        )

        atexit.register(self.exit_application)

    def exit_application(self):
        self.application_running=False
    def set_conf(self,aTable:str):
        try:
            self.iDb.connect()
            if self.iDb is None:
                err=f"Could not connect to bd in Base.set_conf function"
                _logger.error(err)
                pass
            iParams=[str(self.iIdPin)]
            iSql=f"SELECT * FROM {aTable} WHERE idpin=%s"
            self.iDb.execute(aQuery=iSql,aParams=iParams)
            iRet=self.iDb.fetchdata()
            self.iDb.close()
            if iRet == [] or not isinstance(iRet,list):
                err="Could not fetch data in Base.set_conf function"
                err+="\n"
                err+="Function response: "+str(iRet)
                _logger.error(err)
                raise Exception(err)
            
            iRet=iRet[0]
            self.iName=iRet["name"]
            self.iDesc=iRet["description"]
            self.iIsVirtual=iRet["isvirtual"]
            self.iType=iRet["type"]
            self.iIO=iRet["io"] if "io" in iRet else None
            self.iIsHist=iRet["hist"]
            self.iHistPeriod=iRet["histperiod"]
            if self.iIsVirtual:self.run_mode="virtual"
            else:self.run_mode="normal"

        except Exception as e:
            err="Error in Base.set_conf function. Error: "+str(e)+" : "+str(sys.exc_info())
            _logger.error(err)


class DigitalPin(Base):
    def __init__(self
                 ,aBaseDir:str
                 ,aDbInfo:dict
                 ,aPin:str
                 ,aName:str
                 ,aDesc:str
                 ,aIsVirtual:bool
                 ,aType:str
                 ,aIO:str
                 ,aIsHist:bool
                 ,aHistPeriod:int
                 ):
        super().__init__(aBaseDir
                        ,aDbInfo
                        ,aPin
                        ,aName
                        ,aDesc
                        ,aIsVirtual
                        ,aType
                        ,aIO
                        ,aIsHist
                        ,aHistPeriod
                        )
        self.set_conf(aTable="digitalpin")
        self.hist=Historify(self,aTable="historify")
        self.init_digital_thread()
        _logger.info(f"Digital Pin {self.iIdPin} running in {self.run_mode} mode")

    def init_digital_thread(self):
        iTarget=self.main_digital_thread
        iThreadName="Thread_digital_"+str(self.iIdPin)
        self.iDigitalThread=threading.Thread(target=iTarget,name=iThreadName,daemon=True)
        self.iDigitalThread.start()

    def main_digital_thread(self):
        while True: 
            if self.iType=="master":
                 # start another thread to check if it's on for three consecutive times in 5 seconds
                 iTarget=self.master_thread
                 iThreadName="Thread_digital_master"+str(self.iIdPin)
                 iMasterThread=threading.Thread(target=iTarget,name=iThreadName,daemon=True)
                 iMasterThread.start()
            if self.iIsHist:
                self.hist.update_hist()
                wait(seconds=int(self.iHistPeriod))
            else: wait(seconds=10)
    def master_thread(self):
        iMasterTimeLimit=5 # Number of seconds to wait
        running=True
        iCounter=0
        iLastTime=GetEpochTimestamp()
        while running:
            iTime=GetEpochTimestamp()
            time_diff=TimestampTimeDiff(iLastTime,iTime)
            if time_diff<iMasterTimeLimit:
                if iCounter>=3:
                    # AQUI S'HAURIA D'ENCENDRE UN RELÉ
                    running=False
                    break
                else:
                    iValue=int(self.read()["state"])
                    if iValue==1:iCounter+=1
            else:
                wait(seconds=1)
            iLastTime=iTime
        if self.iIsHist:
            # historifico el pin digital
            self.hist.add_hist(new_pin_state=1)

    def read(self):
        try:
            iDir=f"{self.unipi_sys_base_dir}{self.iIdPin}/value"
            if os.path.exists(iDir):
                status = subprocess.run(["cat", f"{self.unipi_sys_base_dir}{self.iIdPin}/value"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                
                if status.returncode != 0:
                    err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
                    _logger.error(err)
                    raise Exception(err)
                else:
                    data={"id":self.iIdPin,"state":str(status.stdout.decode().strip())}
                    return data
            else:
                raise Exception(f"Trying to read {iDir}, but it does not exist")

        except Exception as e:
            err="Could not read pin --{pin}-- value : DigitalPin.read func : Error --> ".format(pin=self.iIdPin)
            err+=str(e)+" : "+str(sys.exc_info())
            _logger.error(err)
            raise Exception(err)


class Relay(Base):
    def __init__(self
                ,aBaseDir:str
                ,aDbInfo:dict
                ,aPin:str
                ,aName:str
                ,aDesc:str
                ,aIsVirtual:bool
                ,aType:str
                ,aIsHist:bool
                ,aHistPeriod:int):
        super().__init__(
                        aBaseDir
                        ,aDbInfo
                        ,aPin
                        ,aName
                        ,aDesc
                        ,aIsVirtual
                        ,aType
                        ,aIsHist
                        ,aHistPeriod)
        self.set_conf(aTable="relay")
        self.calendar=Calendar(self,aTable="relay")
        self.hist=Historify(self,aTable="historify")
        self.init_relay_thread()
        _logger.info(f"Relay {self.iIdPin} running in {self.run_mode} mode")

    def init_relay_thread(self):
        iTarget=self.main_update_thread
        iThreadName="Thread_relay_"+str(self.iIdPin)
        self.iRelayThread=threading.Thread(target=iTarget,name=iThreadName,daemon=True)
        self.iRelayThread.start()
    
    def main_update_thread(self):
        while self.application_running:
            self.update_relay_state()
            self.calendar.update_data()
            if self.iIsHist:
                self.hist.update_hist()
                wait(seconds=self.iHistPeriod)
            else:wait(seconds=5)

    def read(self):
        """
        This function should not be used because it is unreliable

        Sometimes the unipi driver doesn't update the file
        """
        try:
            status = subprocess.run(["cat", f"{self.unipi_sys_base_dir}{self.iIdPin}/value"], stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            if status.returncode != 0:
                err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
                _logger.error(err)
                raise Exception(err)
            else:
                data={"id":self.iIdPin,"state":str(status.stdout.decode().strip())}
                return data
        except Exception as e:
            err="Could not read pin --{pin}-- value : Relay.read func : Error --> ".format(pin=self.iIdPin)
            err+=str(e)+" : "+str(sys.exc_info())
            _logger.error(err)
            raise Exception(err)
    
    def write(self,newState:int):
        """
        Change a relay's state on or off

        newState: 1 (on), 0 (off)

        """

        " echo 1 | sudo tee value"
        status = subprocess.run(f"echo {str(newState)} | sudo tee {self.unipi_sys_base_dir}{self.iIdPin}/value", 
                        shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

        if status.returncode != 0:
            err = f"Error {status.returncode} : {status.stderr.decode().strip()}"
            _logger.error(err)
            return err
        else:
            data={"status":200}
            return data
    
    def update_relay_state(self):
        """
        Start the relay if specified in calendar
        """
        if self.forced_state:
            return
        if not self.calendar.isActive or (self.calendar.startDate is None and self.calendar.endDate is None):
            return
        
        now=GetTime()
        if now >= self.calendar.startDate and now <= self.calendar.endDate:
            self.write(newState=1)
        elif now < self.calendar.startDate or now > self.calendar.endDate:
            self.write(newState=0)

class Calendar:
    def __init__(self,aInstance:Relay|DigitalPin,aTable:str):
        self.iUnit=aInstance
        self.isActive=False
        self.startDate=None
        self.endDate=None
        self.calendarInfo:dict={}
        self.iTable=aTable
        self.update_data()
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
    def update_data(self):
        try:
            """
            Start the pin at a specific date
            """
            self.iUnit.iDb.connect()
            if self.iUnit.iDb is None:
                err=f"Could not connect to bd in Relay.Calendar.{self.update_data.__name__} function"
                _logger.error(err)
                pass
            iSql=f"SELECT * FROM {self.iTable} WHERE idpin='{str(self.iUnit.iIdPin)}'"
            self.iUnit.iDb.execute(aQuery=iSql)
            iRet=self.iUnit.iDb.fetchdata()
            self.iUnit.iDb.close()
            if iRet == [] or not isinstance(iRet,list):
                err="Could not fetch data : "+str(self.update_data.__name__+" function")
                err+="\n"
                err+=str(self.iUnit.iDb.fetchdata.__name__)+" function response: "+str(iRet)
                _logger.error(err)
                raise Exception(err)
            
            self.calendarInfo["calendar"]=iRet[0]
            self.set_data()
            
        except Exception as e:
            _logger.error(f"Error in {str(self.update_data.__name__)} : {str(sys.exc_info())} : {str(e)}")
    def insert_new_data(self):
            # {'iObj':{
            #   'pin': '2.1'
            #   ,'calendar':{
            #       'start_date': '2023-12-20T15:29'
            #       ,'end_date': '2025-05-28T00:00'
            #       }
            #   }
            # }
        try:
            self.iUnit.iDb.connect()
            if self.iUnit.iDb is None:
                err="Could not connect to bd in"
                err+=" Relay.Calendar.insert_new_data"
                err+=" function"
                _logger.error(err)
                pass

            iId=self.iUnit.iIdPin
            iStartDate=self.startDate
            iEndDate=self.endDate
            iActive=self.isActive
            iParams=[iStartDate,iEndDate,iActive,iId]
            iSql="UPDATE {aTable} SET start_date=%s,end_date=%s,date_active=%s WHERE idpin = %s".format(aTable=self.iTable)
            self.iUnit.iDb.execute(iSql,iParams)
            self.iUnit.iDb.close()
        except Exception as e:
            _logger.error(f"Error in {str(self.insert_new_data.__name__)} : {str(sys.exc_info())} : {str(e)}")

class Historify:
    """
    Historify changes in pin states
    """
    def __init__(self,aInstance:Relay|DigitalPin,aTable:str):
        self.iTable=aTable
        self.iUnit=aInstance
        self.last_pin_state=None
    def update_hist(self):
        try:
            if self.last_pin_state is None:
                self.last_pin_state=self.iUnit.read()
                self.last_pin_state=self.last_pin_state["state"]
            read_pin=self.iUnit.read()
            pin_state=read_pin["state"]
            if pin_state!=self.last_pin_state:
                self.add_hist(new_pin_state=pin_state)
                self.last_pin_state=pin_state
        except Exception as e:
            err="Historify.main_hist : "+str(sys.exc_info)+" : "+str(e)
            _logger.error(err)
    def add_hist(self,new_pin_state:str):
        try:
            self.iUnit.iDb.connect()
            iTs=GetTime(aTimeZone="Europe/Madrid",accuracy="s")
            iIdPin=str(self.iUnit.iIdPin)
            iNewState=str(new_pin_state)
            iTs=str(iTs)
            iParams=[iIdPin,iNewState,iTs]
            iSql="INSERT INTO `{}` (idpin,newstate,ts) VALUES (%s,%s,%s)".format(str(self.iTable))
            self.iUnit.iDb.execute(iSql,iParams)
            self.iUnit.iDb.close()
        except Exception as e:
            err="Error in Relay.Historify.add_hist function : "+str(sys.exc_info())+" : "+str(e)
            _logger.error(err)

class BaseHandler:
    def __init__(self):
        self.iJsonFile="/opt/home-auto/home-auto-web/config/conf.json"
        self.unipi_sys_base_dir=""
        self.iDbInfo={}

        self.init_vars()
    def init_vars(self):
        try:
            iDic=get_json_data(aJsonFileDir=self.iJsonFile)
            self.unipi_sys_base_dir=iDic["config"]["unipi_sys_base_dir"]
            db_info=iDic["config"]["database_info"]
            self.iDbInfo={
                "Host":db_info["Host"]
                ,"User":db_info["User"]
                ,"Password":db_info["Password"]
                ,"DataBase":db_info["DataBase"]
            }
        except Exception as e:
            err="Could not get config data from config directory : "
            err+=str(e)+" : "+str(sys.exc_info())

    def get_device_data(self,aTable:str):
        iDb=DataBase(
            Host=self.iDbInfo["Host"]
            ,User=self.iDbInfo["User"]
            ,Password=self.iDbInfo["Password"]
            ,DataBase=self.iDbInfo["DataBase"]
            ,buffered=True
        )
        iDb.connect()
        if iDb is None:
            err=f"Could not connect to bd in BaseHandler.get_device_data function"
            _logger.error(err)
            pass
        iSql=f"SELECT * FROM {aTable}"
        iDb.execute(aQuery=iSql)
        iRet=iDb.fetchdata()
        iDb.close()
        if iRet == [] or not isinstance(iRet,list):
            err="Could not fetch data in BaseHandler.get_device_data function"
            err+="\n"
            err+="Function response: "+str(iRet)
            _logger.error(err)
            raise Exception(err)
            
        return iRet

class RelayHandler(BaseHandler):
    def __init__(self):
        super().__init__()
        self.relays={}
        self.set_relays()
    def set_relays(self):
        iTable="relay"
        iData=self.get_device_data(aTable=iTable) # all db registers of relay table
        for i in range(len(iData)):
            rl_info=iData[i] # db register for each ipin
            self.relays[rl_info["idpin"]]=Relay(aBaseDir=self.unipi_sys_base_dir
                                                ,aDbInfo=self.iDbInfo
                                                ,aPin=rl_info["idpin"]
                                                ,aName=rl_info["name"]
                                                ,aDesc=rl_info["description"]
                                                ,aIsVirtual=rl_info["isvirtual"]
                                                ,aType=rl_info["type"]
                                                ,aIsHist=rl_info["hist"]
                                                ,aHistPeriod=rl_info["histperiod"]
                                                )
    
    def get_relay(self,aPin:str):
        try:
            return self.relays[aPin]
        except Exception as e:
            err="Error in RelayHandler.get_relay function trying to get relay "
            err+=str(aPin)
            err+=": Error: "
            err+=str(e)+" : "
            err+=str(sys.exc_info())
            _logger.error(err)
        match aPin:
            case "2.1":return self.relays["RO2.1"]
            case "2.2":return self.relays["RO2.2"]
            case "2.3":return self.relays["RO2.3"]
            case "2.4":return self.relays["RO2.4"]
            case "2.5":return self.relays["RO2.5"]
            case "2.6":return self.relays["RO2.6"]
            case "2.7":return self.relays["RO2.7"]
            case "2.8":return self.relays["RO2.8"]
        return None

class DigitalPinHandler(BaseHandler):
    def __init__(self):
        super().__init__()
        self.dgpins={}
        self.set_dgpins()
    def set_dgpins(self):
        iTable="digitalpin"
        iData=self.get_device_data(aTable=iTable)
        for i in range(len(iData)):
            dp_info=iData[i]
            self.dgpins[dp_info["idpin"]]=DigitalPin(aBaseDir=self.unipi_sys_base_dir
                                                    ,aDbInfo=self.iDbInfo
                                                    ,aPin=dp_info["idpin"]
                                                    ,aName=dp_info["name"]
                                                    ,aDesc=dp_info["description"]
                                                    ,aIsVirtual=dp_info["isvirtual"]
                                                    ,aType=dp_info["type"]
                                                    ,aIO=dp_info["io"]
                                                    ,aIsHist=dp_info["hist"]
                                                    ,aHistPeriod=dp_info["histperiod"]
                                                    )
 
    def get_digitalpin(self,aPin:str):
        try:
            return self.dgpins[aPin]
        except Exception as e:
            err="Error in DigitalPinHandler.get_digitalpin function trying to get pin "
            err+=str(aPin)
            err+=": Error: "
            err+=str(e)+" : "
            err+=str(sys.exc_info())
            _logger.error(err)
        match aPin:
            case "2.1":return self.dgpins["DI2.1"]
            case "2.2":return self.dgpins["DI2.2"]
            case "2.3":return self.dgpins["DI2.3"]
            case "2.4":return self.dgpins["DI2.4"]
            case "2.5":return self.dgpins["DI2.5"]
            case "2.6":return self.dgpins["DI2.6"]
            case "2.7":return self.dgpins["DI2.7"]
            case "2.8":return self.dgpins["DI2.8"]
        return None

