<?php
require_once "util.php";
require_once "connection.php";

function get_all_devices(){
	// 1. Create xml
	$requestXML = get_all_devices_xml();

	// 2. Send xml to server
	$sock_fd = server_connect();
	server_send($sock_fd, $requestXML);

	// 3. Read response xml and set devices
	$response = server_read($sock_fd);
	write2console("Server response:\\n".$response);

	$out = "<script>";
	$out .= "set_devices(\"".$response."\")";
	$out .= "</script>";
	echo $out;

	server_close($sock_fd);

	return 0;
}

function get_all_devices_xml(){
	$xml = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	$xml .= "<request>\n";
	$xml .= "<function>get_all_devices</function>\n";
	$xml .= "</request>";

	return $xml;
}

function insert_svg($svg_path){
	$svg_file = fopen($svg_path, "r") or die(write2console("Error: could not open ".$svg_path." file."));
	echo fread($svg_file, filesize($svg_path));
	fclose($svg_file);
}

?>
