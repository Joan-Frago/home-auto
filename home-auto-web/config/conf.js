let api_ip = "";
let api_port = "";

async function setData() {
  return fetch('http://127.0.0.1/home/config/conf.json')
    .then((response) => response.json())
    .then((config) => {
      api_ip = config.config.api_ip;
      api_port = config.config.api_port;
    })
    .catch((error) => {
      console.error("Error fetching config json file: ", error);
    })
}

function ConfPin(event){
  event.preventDefault();

  const iForm = document.getElementById("conf-form");
  const iData = new FormData(iForm);
  let iDevice = iData.get("iDevice");
  let iPin = iData.get("iPin"); // attr name of the input
  let iType = iData.get("iType");
  let iName = iData.get("iName");
  let iDesc = iData.get("iDesc");
  let iIsVirtual = iData.get("isVirtual");
  let iIsHist = iData.get("isHist");
  let iHistPeriod = iData.get("iHistPeriod");
  let iIO = iData.get("iIO");

  if (iDevice=="relay") iDevice="RO";
  else if (iDevice=="digitalpin") iDevice="DI";

  if (iIsVirtual=="on") iIsVirtual="1";
  else iIsVirtual="0";

  if (iIsHist=="on") iIsHist="1";
  else iIsHist="0";

  let iIdPin=iDevice+iPin;

  let iObj = {
    idpin: iIdPin
    ,type: iType
    ,name: iName
    ,description: iDesc
    ,isvirtual: iIsVirtual
    ,ishist: iIsHist
    ,histperiod: iHistPeriod
    ,io: iIO
  };
  console.log(iObj);

  let iRoute=`http://${api_ip}:${api_port}/api/SetConf`;
  fetch(iRoute, {
    method: "POST"
    ,headers: {
      "Content-Type": "application/json"
    },
    body: JSON.stringify({ iObj })
  })
  .then(response => {
    if (!response.ok) throw new Error(`HTTP error. status: ${response.status}`);
    return response.json();
  })
  .catch(error => {
    const container = document.getElementById("general-error");
    container.innerHTML=`
      <p class="error-message">Failed in api call. Please try again later.</p>
      <p class="error-message">Error:</p><p>${error}</p>
    `;
    console.error("Error calling api to update config:", error);
  })
}

setData();

