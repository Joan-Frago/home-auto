<?php
require_once "util.php";
error_reporting(E_ALL);

function server_connect(){
	write2console("Connecting to TCP server...");

	$address = 'msi';
	//$address = 'home-unipi';
	$service_port = 8080;

	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
	if ($socket === false) {
		write2console("socket_create() failed: reason: " . socket_strerror(socket_last_error()));
		return -1;
	}

	$result = socket_connect($socket, $address, $service_port);
	if ($result === false) {
		write2console("socket_connect() failed. Reason: ($result) " . socket_strerror(socket_last_error($socket)));
		return -1;
	}
	write2console("Connected to server on ".$address.":".$service_port);

	return $socket;
}

function server_close($socket){
	if($socket){
		socket_close($socket);
	}
	write2console("Closed connection with server.");
	return 0;
}

function server_send($socket, $buf){
	$ret = socket_write($socket, $buf, strlen($buf));
	if($ret === false){
		write2console("Error sending message to server.");
		write2console(socket_strerror(socket_last_error($socket)));
		return -1;
	}

	return 0;
}

function server_read($socket){
	$out = socket_read($socket, 4096);

	if($out == false){
		write2console("Error reading or server disconnected");
		return -1;
	}
	return $out;
}

?>
