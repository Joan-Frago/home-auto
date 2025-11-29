
struct Relay{
	char *id_pin; // e.g. RO2.1
	char *pin; // e.g. 2.1
};

int relay_write(struct Relay *, int);


struct DigitalInput{
	char *id_pin;
	char *pin;
};

int digital_read(struct DigitalInput *, int *);
