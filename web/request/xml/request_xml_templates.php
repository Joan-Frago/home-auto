<?php
function get_all_devices_xml(){
	$xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	$xml .= "<request>\n";
	$xml .= "<function>get_all_devices</function>\n";
	$xml .= "</request>";

	return $xml;
}
function get_device_xml($data){
	$xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	$xml .= "<request>\n";
	$xml .= "<function>get_device</function>\n";
	$xml .= "<data>".$data."</data>";
	$xml .= "</request>";

	return $xml;
}
function update_pin_state_xml($data){
	$xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	$xml .= "<request>\n";
	$xml .= "<function>update_pin_state</function>\n";
	$xml .= "<data>".$data."</data>";
	$xml .= "</request>";

	return $xml;
}
?>
