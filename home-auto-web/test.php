<?php
error_reporting(E_ALL);

echo "Trying to connect to TCP server\n";

$address = 'localhost';
$service_port = 8080;

$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
if ($socket === false) {
	echo "socket_create() failed: reason: " . socket_strerror(socket_last_error()) . "\n";
} else {
	echo "OK.\n";
}

echo "Attempting to connect to '$address' on port '$service_port'...";
$result = socket_connect($socket, $address, $service_port);
if ($result === false) {
	echo "socket_connect() failed.\nReason: ($result) " . socket_strerror(socket_last_error($socket)) . "\n";
} else {
	echo "OK.\n";
}

$in = "new_device\n\n";
$in .= "device_name=Device 1\n";
$in .= "relay_id_pin=RO2.1\n";
$in .= "relay_id=2.1\n";
$in .= "digitalinput_id_pin=DI1.4\n";
$in .= "digitalinput_id=1.4\n\n";
$in .= "\n";

echo "Sending message...";
socket_write($socket, $in, strlen($in));
echo "OK.\n";

echo "Reading response:\n";
$out = '';
$out = socket_read($socket, 2048);

if($out == false){
	echo "Error reading or server disconnected\n";
}
echo $out;

echo "Closing socket...";
if($socket){
	socket_close($socket);
}
echo "OK.\n\n";
?>
