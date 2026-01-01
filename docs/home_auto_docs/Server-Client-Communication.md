# Communication
The communication between the client and the server (unipi) will use xml.

To keep it simple, the client will have the following possibilities:
1. ```get_all_devices``` Ask for all devices
2. ```set_device``` Modify a specific device

Accordingly, the server will have the following endpoints:
1. ```get_all_devices``` Will get the whole xml and send it back
2. ```set_device``` Will modify the xml for a device and send back the new xml

