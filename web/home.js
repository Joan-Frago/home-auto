/*	This work is licensed under Creative Commons GNU LGPL License.

	License: http://creativecommons.org/licenses/LGPL/2.1/
   Version: 0.9
	Author:  Stefan Goessner/2006
	Web:     http://goessner.net/ 
*/
function xml2json(xml, tab) {
	var X = {
		toObj: function(xml) {
			var o = {};
			if (xml.nodeType == 1) {   // element node ..
				if (xml.attributes.length)   // element with attributes  ..
					for (var i = 0; i < xml.attributes.length; i++)
						o["@" + xml.attributes[i].nodeName] = (xml.attributes[i].nodeValue || "").toString();
				if (xml.firstChild) { // element has child nodes ..
					var textChild = 0, cdataChild = 0, hasElementChild = false;
					for (var n = xml.firstChild; n; n = n.nextSibling) {
						if (n.nodeType == 1) hasElementChild = true;
						else if (n.nodeType == 3 && n.nodeValue.match(/[^ \f\n\r\t\v]/)) textChild++; // non-whitespace text
						else if (n.nodeType == 4) cdataChild++; // cdata section node
					}
					if (hasElementChild) {
						if (textChild < 2 && cdataChild < 2) { // structured element with evtl. a single text or/and cdata node ..
							X.removeWhite(xml);
							for (var n = xml.firstChild; n; n = n.nextSibling) {
								if (n.nodeType == 3)  // text node
									o["#text"] = X.escape(n.nodeValue);
								else if (n.nodeType == 4)  // cdata node
									o["#cdata"] = X.escape(n.nodeValue);
								else if (o[n.nodeName]) {  // multiple occurence of element ..
									if (o[n.nodeName] instanceof Array)
										o[n.nodeName][o[n.nodeName].length] = X.toObj(n);
									else
										o[n.nodeName] = [o[n.nodeName], X.toObj(n)];
								}
								else  // first occurence of element..
									o[n.nodeName] = X.toObj(n);
							}
						}
						else { // mixed content
							if (!xml.attributes.length)
								o = X.escape(X.innerXml(xml));
							else
								o["#text"] = X.escape(X.innerXml(xml));
						}
					}
					else if (textChild) { // pure text
						if (!xml.attributes.length)
							o = X.escape(X.innerXml(xml));
						else
							o["#text"] = X.escape(X.innerXml(xml));
					}
					else if (cdataChild) { // cdata
						if (cdataChild > 1)
							o = X.escape(X.innerXml(xml));
						else
							for (var n = xml.firstChild; n; n = n.nextSibling)
								o["#cdata"] = X.escape(n.nodeValue);
					}
				}
				if (!xml.attributes.length && !xml.firstChild) o = null;
			}
			else if (xml.nodeType == 9) { // document.node
				o = X.toObj(xml.documentElement);
			}
			else
				alert("unhandled node type: " + xml.nodeType);
			return o;
		},
		toJson: function(o, name, ind) {
			var json = name ? ("\"" + name + "\"") : "";
			if (o instanceof Array) {
				for (var i = 0, n = o.length; i < n; i++)
					o[i] = X.toJson(o[i], "", ind + "\t");
				json += (name ? ":[" : "[") + (o.length > 1 ? ("\n" + ind + "\t" + o.join(",\n" + ind + "\t") + "\n" + ind) : o.join("")) + "]";
			}
			else if (o == null)
				json += (name && ":") + "null";
			else if (typeof (o) == "object") {
				var arr = [];
				for (var m in o)
					arr[arr.length] = X.toJson(o[m], m, ind + "\t");
				json += (name ? ":{" : "{") + (arr.length > 1 ? ("\n" + ind + "\t" + arr.join(",\n" + ind + "\t") + "\n" + ind) : arr.join("")) + "}";
			}
			else if (typeof (o) == "string")
				json += (name && ":") + "\"" + o.toString() + "\"";
			else
				json += (name && ":") + o.toString();
			return json;
		},
		innerXml: function(node) {
			var s = ""
			if ("innerHTML" in node)
				s = node.innerHTML;
			else {
				var asXml = function(n) {
					var s = "";
					if (n.nodeType == 1) {
						s += "<" + n.nodeName;
						for (var i = 0; i < n.attributes.length; i++)
							s += " " + n.attributes[i].nodeName + "=\"" + (n.attributes[i].nodeValue || "").toString() + "\"";
						if (n.firstChild) {
							s += ">";
							for (var c = n.firstChild; c; c = c.nextSibling)
								s += asXml(c);
							s += "</" + n.nodeName + ">";
						}
						else
							s += "/>";
					}
					else if (n.nodeType == 3)
						s += n.nodeValue;
					else if (n.nodeType == 4)
						s += "<![CDATA[" + n.nodeValue + "]]>";
					return s;
				};
				for (var c = node.firstChild; c; c = c.nextSibling)
					s += asXml(c);
			}
			return s;
		},
		escape: function(txt) {
			return txt.replace(/[\\]/g, "\\\\")
				.replace(/[\"]/g, '\\"')
				.replace(/[\n]/g, '\\n')
				.replace(/[\r]/g, '\\r');
		},
		removeWhite: function(e) {
			e.normalize();
			for (var n = e.firstChild; n;) {
				if (n.nodeType == 3) {  // text node
					if (!n.nodeValue.match(/[^ \f\n\r\t\v]/)) { // pure whitespace text node
						var nxt = n.nextSibling;
						e.removeChild(n);
						n = nxt;
					}
					else
						n = n.nextSibling;
				}
				else if (n.nodeType == 1) {  // element node
					X.removeWhite(n);
					n = n.nextSibling;
				}
				else                      // any other node
					n = n.nextSibling;
			}
			return e;
		}
	};
	if (xml.nodeType == 9) // document node
		xml = xml.documentElement;
	var json = X.toJson(X.toObj(X.removeWhite(xml)), xml.nodeName, "\t");
	return "{\n" + tab + (tab ? json.replace(/\t/g, tab) : json.replace(/\t|\n/g, "")) + "\n}";
}


classes = {
	INVISIBLE:  "invisible",
	BUTTON_ON:  "on",
	BUTTON_OFF: "off"
};

texts = {
	BUTTON_ON:  "On",
	BUTTON_OFF: "Off"
};

let devices = [];

function get_local_device(id){
	return devices.find(d => d["@id"] == id);
}

async function set_all_devices(){
	// loop through devices in the document and get their config
	const devices_arr = document.querySelectorAll("section.device");

	devices_arr.forEach(async (device) => {
		let data = `<device id=\"${device.id}\"></device>`;
	
		let body = {
			xml_function: "get_device_xml",
			has_data: true,
			data: data
		};

		try {
			const response = await fetch("./request/make_request.php", {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json',
				},
				body: JSON.stringify(body)
			});

			if(!response.ok){
				throw new Error("In \"set_all_devices\" function: Could not fetch devices pin status");
			}

			const data = await response.text();

			// execute js function here with the data returned by the server.
			let parser = new DOMParser();
			let device_xml_doc = parser.parseFromString(data, "application/xml");

			let device_json = xml2json(device_xml_doc, "");
			device_json = JSON.parse(device_json);

			if(is_error(device_json)){
				console.error(device_json.error);
				return;
			}

			set_device(device_json.device);

		} catch (error) {
			console.error("Error: ", error);
		}
	});
}

function set_device(device) {
	document.getElementById(device["@id"]+"_title").textContent = device.name;
	document.getElementById(device["@id"]+"_modal-title").textContent = device.name;
	document.getElementById(device["@id"]+"_modal-description").textContent = device.description;

	construct_device(device);

	const modal = document.getElementById(device["@id"]+"_modal");
	device.svg.onclick = () => {
		modal.style.display = "flex";
	};

	const modal_close = document.getElementById(device["@id"]+"_modal-close");
	modal_close.onclick = () => {
		modal.style.display = "none";
	};

	window.onclick = (event) => {
		if(event.target == modal){
			modal.style.display = "none";
		}
	};

	devices.push(device);
}

async function update_pin_state(device){
	if(!device.has_rl() || !device.has_di()){
		console.log("Cannot change the state of \""+device.name+"\" device");
		return;
	}
	let new_state = 0;
	if(device.digital_input["@value"] == 0){
		new_state = 1;
	}

	let data = `<device id=\"${device["@id"]}\" new_state=\"${new_state}\"></device>`;

	let body = {
		xml_function: "update_pin_state_xml",
		has_data: true,
		data: data
	};

	try {
		const response = await fetch("./request/make_request.php", {
			method: 'POST',
			headers: {
				'Content-Type': 'application/json',
			},
			body: JSON.stringify(body)
		});

		if(!response.ok){
			throw new Error("In \"update_pin_state\" function: Error in update_pin_state request.");
		}

		const data = await response.text();

		// execute js function here with the data returned by the server.
		// example:
		// set_devices(data)

		let parser = new DOMParser();
		let xml_doc = parser.parseFromString(data, "application/xml");

		let json = xml2json(xml_doc, "");
		json = JSON.parse(json);

		if(is_error(json)){
			console.error(json.error);
			return;
		}

		console.log(json);
		console.log(device);

	} catch (error) {
		console.error("Error: ", error);
	}
}

async function get_device_pin_status(){

	// loop through devices in the document and get their actual values
	const devices_arr = document.querySelectorAll("section.device");

	devices_arr.forEach(async (device) => {
		//let id = 1;
		let data = `<device id=\"${device.id}\"></device>`;

		let body = {
			xml_function: "get_device_xml",
			has_data: true,
			data: data
		};

		try {
			const response = await fetch("./request/make_request.php", {
				method: 'POST',
				headers: {
					'Content-Type': 'application/json',
				},
				body: JSON.stringify(body)
			});

			if(!response.ok){
				throw new Error("In \"get_device_pin_status\" function: Could not fetch devices pin status");
			}

			const data = await response.text();

			// execute js function here with the data returned by the server.
			// example:
			// set_devices(data)

			let parser = new DOMParser();
			let xml_doc = parser.parseFromString(data, "application/xml");

			let json = xml2json(xml_doc, "");
			json = JSON.parse(json);

			if(is_error(json)){
				console.error(json.error);
				return;
			}

			construct_device(json.device);
			update_device(json.device);

		} catch (error) {
			console.error("Error: ", error);
		}
	});
}

function is_error(obj){
	return obj.hasOwnProperty("error");
}

function construct_device(device){
	device.has_rl = () => device.hasOwnProperty("relay");
	device.has_di = () => device.hasOwnProperty("digital_input");
	device.has_mb = () => device.hasOwnProperty("modbus");
	device.svg = document.getElementById(device["@id"] + "_" + device["@type"]);
}

function update_device(device){
	update_device_pin_status(device);
	update_device_svg(device);
}

function update_device_pin_status(device){
	const local_device = get_local_device(device["@id"]);

	if(device.has_di()){
		local_device.digital_input["@value"] = device.digital_input["@value"];
	}

	if(device.has_rl()){
		local_device.relay["@value"] = device.relay["@value"];
	}

	if(device.has_mb()){
		const modal_tbody = document.getElementById(device["@id"]+"_modal-table-tbody");
		let html = "";
 		for(reg of device.modbus.register){
 			html += "<tr>";
			html += "    <td>"+reg["@name"]   + "</td>";
			html += "    <td>"+reg["@value"]  + "</td>";
			html += "    <td>"+reg["@symbol"] + "</td>";
			html += "    <td>"+reg["@line"]   + "</td>";
			html += "</tr>";
 		}
 		modal_tbody.innerHTML = html;
	}
}

function update_device_svg(device){
	// get device svg ( id device + device type )
	const svg = document.getElementById(device["@id"] + device["@type"]);

	// depending of its type execute a function that updates that type of svg
	switch(device["@type"]){
		case "BLIND": 		update_svg_blind(device);      break;
		case "LIGHT_BULB": 	update_svg_light_bulb(device); break;
		case "ANALYZER":	update_svg_analyzer(device);   break;
		default: break;
	}
}

function update_svg_blind(device){
	if(device.has_rl()){
		if(device.relay["@value"] == 1){
			let blank = document.getElementById(device["@id"] + "_blank");
			blank.classList.remove(classes.INVISIBLE);
		}
		else {
			let blank = document.getElementById(device["@id"] + "_blank");
			blank.classList.add(classes.INVISIBLE);
		}
	}
}

function update_svg_light_bulb(device){
	const bulb_glow  = document.getElementById(device["@id"]+"_bulb_glow");
	const bulb_color = document.getElementById(device["@id"]+"_path");
	
	if(device.has_di()){
		if(device.digital_input["@value"] == 1){
			bulb_glow.classList.remove(classes.INVISIBLE);
			bulb_color.setAttribute("fill", "url(#"+device["@type"]+"_color)");
		}
		else {
			bulb_glow.classList.add(classes.INVISIBLE);
			bulb_color.setAttribute("fill", "#FFF");
		}
	}
}

function update_svg_analyzer(device){
	const fields = document.querySelectorAll(".dataField");
	fields.forEach((f) => {
		if(f.id == device["@id"]+"_"+device["@type"]+"_field1_value"){
			f.textContent = device.modbus.register[0]["@value"] / 10;
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field1_symbol"){
			f.textContent = device.modbus.register[0]["@symbol"];
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field1_line"){
			f.textContent = device.modbus.register[0]["@line"];
		}


		if(f.id == device["@id"]+"_"+device["@type"]+"_field2_value"){
			f.textContent = device.modbus.register[1]["@value"] / 10;
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field2_symbol"){
			f.textContent = device.modbus.register[1]["@symbol"];
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field2_line"){
			f.textContent = device.modbus.register[1]["@line"];
		}


		if(f.id == device["@id"]+"_"+device["@type"]+"_field3_value"){
			f.textContent = device.modbus.register[2]["@value"] / 10;
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field3_symbol"){
			f.textContent = device.modbus.register[2]["@symbol"];
		}
		if(f.id == device["@id"]+"_"+device["@type"]+"_field3_line"){
			f.textContent = device.modbus.register[2]["@line"];
		}
	});
}

function load_pin_data(){
	setInterval(get_device_pin_status, 3000);
}


