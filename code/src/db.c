#include "../inc/logger.h"
#include "../inc/config.h"
#include <string.h>
#include <stdlib.h>
#include <libpq-fe.h>

int db_log(int device_id, const char *register_type, const char *register_id, float value){
	// 1. Establish connection
	char conninfo[512];
	sprintf(
		conninfo,
		"dbname=%s user=%s password=%s host=%s",
		get_var_value(POSTGRES_DB_NAME),
		get_var_value(POSTGRES_USER),
		get_var_value(POSTGRES_PASSWORD),
		get_var_value(POSTGRES_HOST)
	);

	PGconn *conn = PQconnectdb(conninfo);

	// Check if connection succeeded
	if (PQstatus(conn) != CONNECTION_OK) {
		LOG_ERROR("Connection failed: %s", PQerrorMessage(conn));
		PQfinish(conn);
		return -1;
	}

	// 2. Prepare Data for Insertion
	char dev_id_str[8];
	char value_str[32];

	snprintf(dev_id_str, sizeof(dev_id_str), "%d", device_id);
	snprintf(value_str, sizeof(value_str), "%f", value);

	const char *paramValues[4];
	paramValues[0] = dev_id_str;
	paramValues[1] = register_type;
	paramValues[2] = register_id;
	paramValues[3] = value_str;

	// 3. Execute Insert
	PGresult *res = PQexecParams(conn,
			"INSERT INTO home_log (device_id, register_type, register_id, value) "
			"VALUES ($1, $2, $3, $4)",
			4,       // number of parameters
			NULL,    // let the backend derive param types
			paramValues,
			NULL,    // text format
			NULL,    // text format
			0        // ask for text results
		);

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
		LOG_ERROR("Insert failed: %s", PQerrorMessage(conn));
	}

	// 4. Cleanup
	PQclear(res);
	PQfinish(conn);

	return 0;
}
