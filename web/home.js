async function get_all_devices(){
	

	let body = {
		xml_function: "get_all_devices_xml"
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
			throw new Error("In \"get_all_devices\" function: Could not fetch devices pin status");
		}

		const data = await response.text();

		// execute js function here with the data returned by the server.
		set_devices(data);

		//console.log(data);
	} catch (error) {
		console.error("Error: ", error);
	}
}

function set_devices(devices_xml_str) {
	let parser = new DOMParser();
	let devices_xml_doc = parser.parseFromString(devices_xml_str, "application/xml");

	let devices_json = xml2json(devices_xml_doc, "");
	console.log(devices_json);

	devices_json = JSON.parse(devices_json);
	console.log(devices_json);
}

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

async function get_device_pin_status(){

	// loop through devices in the document and get their actual values
	const devices_arr = document.querySelectorAll("section.device");

	devices_arr.forEach(async (device) => {
		//let id = 1;
		let data = `<device id=\"${device.id}\"></device>`;

		let body = {
			xml_function: "get_device_pin_status_xml",
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

			console.log("--- get_device_pin_status ---");
			//console.log(data);

			let parser = new DOMParser();
			let xml_doc = parser.parseFromString(data, "application/xml");

			let json = xml2json(xml_doc, "");
			console.log(json);

			json = JSON.parse(json);
			console.log(json);

			update_device_pin_status(json);
		} catch (error) {
			console.error("Error: ", error);
		}
	});
}

function update_device_pin_status(device){
	const element = document.getElementById(device.device["@id"]);
	console.log(element);


}

function load_pin_data(){
	setInterval(get_device_pin_status, 3000);
}
