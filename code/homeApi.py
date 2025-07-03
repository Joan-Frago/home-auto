# General modules
import sys

# Personal modules
from homeMain import RelayHandler,DigitalPinHandler

from pyutils.utils import Logger,NormDate,DataBase
from pyutils.api import Api

# Init logger
_logger=Logger(log_path="/opt/home-auto/log/home.log",enable_rotation=False,max_log_file_size=90)
# Unhandled exceptions
sys.excepthook = _logger.exception_handler

def ReadPin(data,aPin:str):
    try:
        if "R" in aPin:
            raise Exception("Can't read the state of a Relay directly")
        iPin=dp_handler.get_digitalpin(aPin)
        iRet=iPin.read()
        if isinstance(iRet,dict):
            return iRet
    except Exception as e:
        err=f"Could not read {aPin} pin. Error: {str(e)} : {str(sys.exc_info())}"
        return {"error":err}

def ReadAllPins():
    try:
        pins_state={
            "pins": []
        }
        # set relays
        for index,idpin in enumerate(dp_handler.dgpins):
            pin=dp_handler.get_digitalpin(idpin)
            pin_status=pin.read()
            pins_state["pins"].append(pin_status)
            pins_state["pins"][index]["name"]=pin.iName
            pins_state["pins"][index]["desc"]=pin.iDesc
            pins_state["pins"][index]["isvirtual"]=pin.iIsVirtual
            pins_state["pins"][index]["type"]=pin.iType
            pins_state["pins"][index]["io"]=pin.iIO
            pins_state["pins"][index]["ishist"]=pin.iIsHist
            pins_state["pins"][index]["histperiod"]=pin.iHistPeriod
            pins_state["pins"][index]["runmode"]=pin.run_mode
            pins_state["pins"][index]["forcedvalue"]=pin.forced_state
        return pins_state

        for i in range(1,9):
            iPin="2."+str(i)
            pin=rl_handler.get_relay(iPin)
            pin_status = pin.read()
            pins_state["pins"].append(pin_status)
        return pins_state
    except Exception as e:
        pass

def WriteRelay(data):
    # s'ha de canviar això. idealment s'hauria de mirar si el que ve
    # és un digitalpin o relay
    # e.g. /api/WriteRelay/digital/1.5
    # e.g. /api/WriteRelay/relay/2.4
    # potser seria millor passar body (data) amb *ipintype* i *ipin*

    iPin=data["iObj"]["pin"]
    iStatus=int(data["iObj"]["newstate"])

    if iStatus != 0 and iStatus != 1:
        return {"error": f"Relay status {iStatus} can't be set: incorrect status form"}
    relay = rl_handler.get_relay(iPin)
    
    relay.forced_state=True
    status=relay.write(newState=iStatus)
    return status

def GetCalendar(data,aPin:str):
    aPin="RO"+aPin
    relay=rl_handler.get_relay(aPin)
    calendar_data=relay.calendar.calendarInfo
    return calendar_data

def SetCalendar(data,aPin:str):
    if not isinstance(data,dict):
        err=f"Cannot insert new calendar data to {aPin}, received data is not dict. Received data is {type(data)}"
        _logger.error(str(err))
        return {"status":500,"error":str(err)}
    
    aPin="RO"+aPin
    relay=rl_handler.get_relay(aPin)
    iDic=data["iObj"]["calendar"]
    cal_active=iDic["is_active"]
    start_date=NormDate(iDic["start_date"])
    end_date=NormDate(iDic["end_date"])

    if cal_active=="1":cal_active=1
    elif cal_active=="0":cal_active=0

    relay.calendar.isActive=cal_active
    relay.calendar.startDate=start_date
    relay.calendar.endDate=end_date
    
    relay.calendar.insert_new_data()

    return {"status":200}

def DisableForcedState(data,aPin:str):
    try:
        aPin="RO"+aPin
        relay=rl_handler.get_relay(aPin=aPin)
        relay.forced_state=False
        return {"status":200}
    except Exception as e:
        err="Error in homeApi.py:DisableForcedState function."
        err+=" Error: "+str(e)
        err+=" : "+str(sys.exc_info())
        _logger.error(err)

def SetConf(data):
    try:
        if not isinstance(data,dict): raise Exception("data is not dict in SetConf function")
        iConf=data["iObj"]
        iIdPin=iConf["idpin"]
        iType=iConf["type"]
        iName=iConf["name"]
        iDesc=iConf["description"]
        iIsVirtual=iConf["isvirtual"]
        iIsHist=iConf["ishist"]
        iHistPeriod=iConf["histperiod"] if iConf["histperiod"]!='' else "0"
        iIO=iConf["io"]

        if "RO" in iIdPin:
            iHandler=rl_handler
            iHandler.get_relay(aPin=iIdPin)
            iTable="relay"
        if "DI" in iIdPin:
            iHandler=dp_handler
            iHandler.get_digitalpin(aPin=iIdPin)
            iTable="digitalpin"

        iDb=DataBase(
            Host=iHandler.iDbInfo["Host"]
            ,User=iHandler.iDbInfo["User"]
            ,Password=iHandler.iDbInfo["Password"]
            ,DataBase=iHandler.iDbInfo["DataBase"]
            ,buffered=True
        )
        iDb.connect()
        if iDb is None:
            err=f"Could not connect to bd in homeApi.py:SetConf function"
            _logger.error(err)
            pass

        iParams=[str(iName),str(iDesc),str(iIsVirtual)
                 ,str(iType),str(iIO),str(iIsHist),str(iHistPeriod)
                 ,str(iIdPin)]

        iSql="UPDATE {aTable} SET".format(aTable=iTable)
        iSql+=" name=%s"
        iSql+=", description=%s"
        iSql+=", isvirtual=%s"
        iSql+=", type=%s"
        iSql+=", io=%s"
        iSql+=", hist=%s"
        iSql+=", histperiod=%s"
        iSql+=" WHERE idpin=%s"

        iQuery = iDb.execute(iSql,iParams,DebugMode=False)
        iDb.close()

        return {"status":200}
    except Exception as e:
        err="Error in homeApi.py:SetConf function."
        err+=" Error: "+str(e)
        err+=" : "+str(sys.exc_info())
        _logger.error(err)
        return {"status":500,
                "error":str(err)}

if __name__ == "__main__":
    rl_handler=RelayHandler()
    dp_handler=DigitalPinHandler()
    _api = Api(Port=8000,logger=_logger
            ,init_message="Started home automation api"
            ,exit_message="Closed home automation api"
            ,allowed_origins=["http://100.116.80.15:8020"
                              ,"http://homeserver:8020"
                              ,"http://100.117.134.68"
                              ,"http://127.0.0.1"])
    _api.add_get_request("/api/ReadAllPins",ReadAllPins)
    _api.add_post_request(r"/api/ReadPin/(?P<aPin>2\.[1-9])",ReadPin)
    _api.add_post_request("/api/WriteRelay",WriteRelay)
    _api.add_post_request(r"/api/GetCalendar/(?P<aPin>2\.[1-9])",GetCalendar)
    _api.add_post_request(r"/api/SetCalendar/(?P<aPin>2\.[1-9])",SetCalendar)
    _api.add_post_request(r"/api/DisableForcedState/(?P<aPin>2\.[1-9])",DisableForcedState)
    _api.add_post_request(r"/api/SetConf",SetConf)
    _api.init_app()
