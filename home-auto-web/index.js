// {
//      "pins":
//           [
//               {"pin":"2.1","state":"0"},
//               {"pin":"2.2","state":"0"},
//               {"pin":"2.3","state":"0"},
//               {"pin":"2.4","state":"1"},
//               {"pin":"2.5","state":"0"},
//               {"pin":"2.6","state":"0"},
//               {"pin":"2.7","state":"0"},
//               {"pin":"2.8","state":"0"}
//           ]
// }

let api_ip="";
let api_port="";

function setData(){
  return fetch('http://127.0.0.1/home-auto-web/config/conf.json')
  .then((response) => response.json())
  .then((config) => {
    api_ip=config.config.api_ip;
    api_port=config.config.api_port;
  })
  .catch((error) => {
    console.error("Error fetching config json file: ",error);
  })
}
  


function fetchPinStates() {
  let iRoute=`http://${api_ip}:${api_port}/api/ReadAllPins`;
  fetch(iRoute)
    .then(response => response.json()) // Convert response to JSON
    .then(data => {
      data.pins.forEach((pin) => {
        // establir estat del pin per cada div
        const p_pin_state = document.getElementById(`p-pin-state-${pin.id}`)
        // afegir atribut
        p_pin_state.setAttribute("class",`id-pin-state-${pin.state.toLowerCase()} p-pin-state`)
        p_pin_state.innerHTML = ""
        p_pin_state.innerHTML = `${pin.state === "1" ? "UP" : "DOWN"}`

      });
    })
    .catch(error => {
      const container = document.getElementById("pins-container");
      container.innerHTML = `
        <p class="error-message">Failed to load pin states. Please try again later.</p>
        <p class="error-message">Error:</p><p>${error}</p>
      `;

      console.error("Error fetching pin states:", error);
    });
}

function fetch_state_change(pin,state) {
  if (state != 0 && state != 1) {
    // not valid
    console.log("Invalid state in fetch_state_change: ",state)
    return;
  }
  let iRoute=`http://${api_ip}:${api_port}/api/WriteRelay/${pin}/${state}`;
  fetch(iRoute, {
    method:"POST",
    headers:{
      "Content-Type": "application/json",
    },
    body: JSON.stringify({}),
  })
    .then(response => {
      if (!response.ok) throw new Error(`HTTP error. status: ${response.status}`);
      fetchPinStates();
      return response.json();
    })
    .then((data) => {
      console.log("Relay state changed successfully:",data);
    })
    .catch(error => {
      const container = document.getElementById(`error-${pin}`);
      container.innerHTML = `
        <p class="error-message">Failed in api call. Please try again later.</p>
        <p class="error-message">Error:</p><p>${error}</p>
      `;

      console.error("Error calling api to change relay state:", error);
    });
}
setData().then(() => {
  fetchPinStates();
  setInterval(fetchPinStates, 600000);
});

