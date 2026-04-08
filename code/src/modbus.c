#include "../inc/unipi_control.h"
#include "../inc/logger.h"
#include "../inc/config.h"

#include <stdlib.h>
#include <modbus.h>
#include <errno.h>

static void add_register(reg_t *, char *, char *, char *, int, char, int);
static float modbus_correct_reg(uint32_t value, reg_t reg);

static void add_register(reg_t *reg, char *name, char *symbol, char *line, int id, char correction_op, int correction_no){
	reg->name   = name;
	reg->symbol = symbol;
	reg->line   = line;
	reg->id     = id;

	reg->correction_op = correction_op;
	reg->correction_no = correction_no;
}

void analyzer_set_registers(reg_t registers[REGISTER_COUNT]){
	add_register(&registers[0],  "Tension Fase L1",    "V",    "L1", 0, '/', 10);
	add_register(&registers[1],  "Corriente L1",       "A",    "L1", 2, '\0', 0);
	add_register(&registers[2],  "Potencia Activa L1", "kW",   "L1", 4, '\0', 0);

	add_register(&registers[3],  "Tension Fase L2",    "V",    "L2", 10, '/', 10);
	add_register(&registers[4],  "Corriente L2",       "A",    "L1", 12, '\0', 0);
	add_register(&registers[5],  "Potencia Activa L2", "kW",   "L1", 14, '\0', 0);

	add_register(&registers[6],  "Tension Fase L3",    "V",    "L3", 20, '/', 10);
	add_register(&registers[7],  "Corriente L3",       "A",    "L1", 22, '\0', 0);
	add_register(&registers[8],  "Potencia Activa L3", "kW",   "L1", 24, '\0', 0);

	add_register(&registers[9],  "Energia Activa",     "kW·h", "",   60, '\0', 0);
	add_register(&registers[10], "Temperatura",        "ºC",   "",   80, '/', 10);
}

float modbus_read(mb_t modbus, reg_t reg){
	modbus_t *mb;
	uint16_t tab_reg[2];

	if(modbus.connection_type == TCP){
		mb = modbus_new_tcp(modbus.tcp_addr, modbus.tcp_port);
	}
	else if(modbus.connection_type == RS485){
		mb = modbus_new_rtu(
			get_var_value(MODBUS_RTU_DEVICE),
			atoi(get_var_value(MODBUS_RTU_BAUD)),
			*get_var_value(MODBUS_RTU_PARITY),
			atoi(get_var_value(MODBUS_RTU_DATABIT)),
			atoi(get_var_value(MODBUS_RTU_STOPBIT))
		);
	}

	modbus_set_slave(mb, modbus.slave);
	modbus_connect(mb);

	if(modbus_read_registers(mb, reg.id, 2, tab_reg) == -1){
		LOG_ERROR("Read failed: %s", modbus_strerror(errno));
		return 0;
	}

	uint32_t __value = (tab_reg[0] << 16) | tab_reg[1];

	float value = modbus_correct_reg(__value, reg);

	modbus_close(mb);
	modbus_free(mb);

	return value;
}

static float modbus_correct_reg(uint32_t value, reg_t reg){
	float res = (float)value;
	switch(reg.correction_op){
		case '*':  res *= reg.correction_no; break;
		case '/':  res /= reg.correction_no; break;
		case '+':  res += reg.correction_no; break;
		case '-':  res -= reg.correction_no; break;
		case '\0': break;
		default:   break;
	}
	return res;
}
