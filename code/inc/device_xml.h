#ifndef DEVICE_XML_H

#include <libxml/tree.h>
#include <libxml/xpath.h>

#define DEVICE_XML_H

#define XML_DEVICES_PATH "conf/devices.xml"

typedef struct DeviceXml{
	xmlDoc *devices_doc;
	xmlNode *root;
	xmlXPathContext *xpath_context;

} device_xml_t;

device_xml_t *open_devices_xml_file(void);
void close_devices_xml_file(device_xml_t *);

xmlNode *find_child_node(xmlNode *parent, xmlChar *node_name);

#endif
