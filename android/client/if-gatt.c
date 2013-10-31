/*
 * Copyright (C) 2013 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <hardware/bluetooth.h>

#include "if-main.h"

const btgatt_interface_t *if_gatt = NULL;

#define MAX_CHAR_ID_STR_LEN (MAX_UUID_STR_LEN + 3 + 11)
#define MAX_SRVC_ID_STR_LEN (MAX_UUID_STR_LEN + 3 + 11 + 1 + 11)
/* How man characters print from binary objects (arbitrary) */
#define MAX_HEX_VAL_STR_LEN 100
#define MAX_NOTIFY_PARAMS_STR_LEN (MAX_SRVC_ID_STR_LEN + MAX_CHAR_ID_STR_LEN \
		+ MAX_ADDR_STR_LEN + MAX_HEX_VAL_STR_LEN + 60)
#define MAX_READ_PARAMS_STR_LEN (MAX_SRVC_ID_STR_LEN + MAX_CHAR_ID_STR_LEN \
		+ MAX_UUID_STR_LEN + MAX_HEX_VAL_STR_LEN + 80)

#define VERIFY_INT_ARG(n, v, err) \
	do { \
		if (n < argc) \
			v = atoi(argv[n]); \
		else { \
			haltest_error(err); \
			return;\
		} \
	} while (0)

#define VERIFY_HEX_ARG(n, v, err) \
	do { \
		if (n < argc) \
			v = strtol(argv[n], NULL, 16); \
		else { \
			haltest_error(err); \
			return;\
		} \
	} while (0)

/* Helper macros to verify arguments of methods */
#define VERIFY_CLIENT_IF(n, v) VERIFY_INT_ARG(n, v, "No client_if specified\n")
#define VERIFY_SERVER_IF(n, v) VERIFY_INT_ARG(n, v, "No server_if specified\n")
#define VERIFY_CONN_ID(n, v) VERIFY_INT_ARG(n, v, "No conn_if specified\n")
#define VERIFY_HANDLE(n, v) VERIFY_HEX_ARG(n, v, "No "#v" specified\n")
#define VERIFY_SERVICE_HANDLE(n, v) VERIFY_HANDLE(n, v)

#define VERIFY_UUID(n, v) \
	do { \
		if (n < argc) \
			gatt_str2bt_uuid_t(argv[n], -1, v); \
		else { \
			haltest_error("No uuid specified\n"); \
			return;\
		} \
	} while (0)

#define VERIFY_SRVC_ID(n, v) \
	do { \
		if (n < argc) \
			str2btgatt_srvc_id_t(argv[n], v); \
		else { \
			haltest_error("No srvc_id specified\n"); \
			return;\
		} \
	} while (0)

#define VERIFY_CHAR_ID(n, v) \
	do { \
		if (n < argc) \
			str2btgatt_char_id_t(argv[n], v); \
		else { \
			haltest_error("No char_id specified\n"); \
			return;\
		} \
	} while (0)

/* Gatt uses little endian uuid */
static const char GATT_BASE_UUID[] = {
	0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
	0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
 * converts gatt uuid to string
 * buf should be at least 39 bytes
 *
 * This function formats 16, 32 and 128 bits uuid
 *
 * returns string representation of uuid
 */
static char *gatt_uuid_t2str(const bt_uuid_t *uuid, char *buf)
{
	int shift = 0;
	int i = 16;
	int limit = 0;
	int j = 0;

	/* for bluetooth uuid only 32 bits */
	if (0 == memcmp(&uuid->uu, &GATT_BASE_UUID,
						sizeof(bt_uuid_t) - 4)) {
		limit = 12;
		/* make it 16 bits */
		if (uuid->uu[15] == 0 && uuid->uu[14] == 0)
			i = 14;
	}

	while (i-- > limit) {
		if (i == 11 || i == 9 || i == 7 || i == 5) {
			buf[j * 2 + shift] = '-';
			shift++;
		}

		sprintf(buf + j * 2 + shift, "%02x", uuid->uu[i]);
		++j;
	}

	return buf;
}

/*
 * Tries to convert hex string of given size into out buffer.
 * Output buffer is little endian.
 */
static void scan_field(const char *str, int len, uint8_t *out, int out_size)
{
}

static void gatt_str2bt_uuid_t(const char *str, int len, bt_uuid_t *uuid)
{
}

/* char_id formating function */
static char *btgatt_char_id_t2str(const btgatt_char_id_t *char_id, char *buf)
{
	char uuid_buf[MAX_UUID_STR_LEN];

	sprintf(buf, "{%s,%d}", gatt_uuid_t2str(&char_id->uuid, uuid_buf),
							char_id->inst_id);
	return buf;
}

/* Parse btgatt_char_id_t */
static void str2btgatt_char_id_t(const char *buf, btgatt_char_id_t *char_id)
{
}

/* service_id formating function */
static char *btgatt_srvc_id_t2str(const btgatt_srvc_id_t *srvc_id, char *buf)
{
	char uuid_buf[MAX_UUID_STR_LEN];

	sprintf(buf, "{%s,%d,%d}", gatt_uuid_t2str(&srvc_id->id.uuid, uuid_buf),
				srvc_id->id.inst_id, srvc_id->is_primary);
	return buf;
}

/* Parse btgatt_srvc_id_t */
static void str2btgatt_srvc_id_t(const char *buf, btgatt_srvc_id_t *srvc_id)
{
}

/* Converts array of uint8_t to string representation */
static char *array2str(const uint8_t *v, int size, char *buf, int out_size)
{
	int limit = size;
	int i;

	if (out_size > 0) {
		*buf = '\0';
		if (size >= 2 * out_size)
			limit = (out_size - 2) / 2;

		for (i = 0; i < limit; ++i)
			sprintf(buf + 2 * i, "%02x", v[i]);

		/* output buffer not enough to hold whole field fill with ...*/
		if (limit < size)
			sprintf(buf + 2 * i, "...");
	}

	return buf;
}

/* Converts btgatt_notify_params_t to string */
static char *btgatt_notify_params_t2str(const btgatt_notify_params_t *data,
								char *buf)
{
	char addr[MAX_ADDR_STR_LEN];
	char srvc_id[MAX_SRVC_ID_STR_LEN];
	char char_id[MAX_CHAR_ID_STR_LEN];
	char value[MAX_HEX_VAL_STR_LEN];

	sprintf(buf, "{bda=%s, srvc_id=%s, char_id=%s, val=%s, is_notify=%u}",
		bt_bdaddr_t2str(&data->bda, addr),
		btgatt_srvc_id_t2str(&data->srvc_id, srvc_id),
		btgatt_char_id_t2str(&data->char_id, char_id),
		array2str(data->value, data->len, value, sizeof(value)),
							data->is_notify);
	return buf;
}

static char *btgatt_unformatted_value_t2str(const btgatt_unformatted_value_t *v,
							char *buf, int size)
{
	return array2str(v->value, v->len, buf, size);
}

static char *btgatt_read_params_t2str(const btgatt_read_params_t *data,
								char *buf)
{
	char srvc_id[MAX_SRVC_ID_STR_LEN];
	char char_id[MAX_CHAR_ID_STR_LEN];
	char descr_id[MAX_UUID_STR_LEN];
	char value[MAX_HEX_VAL_STR_LEN];

	sprintf(buf, "{srvc_id=%s, char_id=%s, descr_id=%s, val=%s value_type=%d, status=%d}",
		btgatt_srvc_id_t2str(&data->srvc_id, srvc_id),
		btgatt_char_id_t2str(&data->char_id, char_id),
		gatt_uuid_t2str(&data->descr_id, descr_id),
		btgatt_unformatted_value_t2str(&data->value, value, 100),
		data->value_type, data->status);
	return buf;
}

/* BT-GATT Client callbacks. */

/* Callback invoked in response to register_client */
static void gattc_register_client_cb(int status, int client_if,
							bt_uuid_t *app_uuid)
{
	char buf[MAX_UUID_STR_LEN];

	haltest_info("%s: status=%d client_if=%d app_uuid=%s\n", __func__,
						status, client_if,
						gatt_uuid_t2str(app_uuid, buf));
}

/* Callback for scan results */
static void gattc_scan_result_cb(bt_bdaddr_t *bda, int rssi, uint8_t *adv_data)
{
	char buf[MAX_ADDR_STR_LEN];

	haltest_info("%s: bda=%s rssi=%d adv_data=%p\n", __func__,
				bt_bdaddr_t2str(bda, buf), rssi, adv_data);
}

/* GATT open callback invoked in response to open */
static void gattc_connect_cb(int conn_id, int status, int client_if,
							bt_bdaddr_t *bda)
{
	char buf[MAX_ADDR_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d, client_if=%d bda=%s\n",
					__func__, conn_id, status, client_if,
					bt_bdaddr_t2str(bda, buf));
}

/* Callback invoked in response to close */
static void gattc_disconnect_cb(int conn_id, int status, int client_if,
							bt_bdaddr_t *bda)
{
	char buf[MAX_ADDR_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d, client_if=%d bda=%s\n",
					__func__, conn_id, status, client_if,
					bt_bdaddr_t2str(bda, buf));
}

/*
 * Invoked in response to search_service when the GATT service search
 * has been completed.
 */
static void gattc_search_complete_cb(int conn_id, int status)
{
	haltest_info("%s: conn_id=%d status=%s\n", __func__, conn_id, status);
}

/* Reports GATT services on a remote device */
static void gattc_search_result_cb(int conn_id, btgatt_srvc_id_t *srvc_id)
{
	char srvc_id_buf[MAX_SRVC_ID_STR_LEN];

	haltest_info("%s: conn_id=%d srvc_id=%s\n", __func__, conn_id,
				btgatt_srvc_id_t2str(srvc_id, srvc_id_buf));
}

/* GATT characteristic enumeration result callback */
static void gattc_get_characteristic_cb(int conn_id, int status,
					btgatt_srvc_id_t *srvc_id,
					btgatt_char_id_t *char_id,
					int char_prop)
{
	char srvc_id_buf[MAX_SRVC_ID_STR_LEN];
	char char_id_buf[MAX_CHAR_ID_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d srvc_id=%s char_id=%s, char_prop=%x\n",
			__func__, conn_id, status,
			btgatt_srvc_id_t2str(srvc_id, srvc_id_buf),
			btgatt_char_id_t2str(char_id, char_id_buf), char_prop);

	/* enumerate next characteristic */
	if (status == 0)
		EXEC(if_gatt->client->get_characteristic, conn_id, srvc_id,
								char_id);
}

/* GATT descriptor enumeration result callback */
static void gattc_get_descriptor_cb(int conn_id, int status,
		btgatt_srvc_id_t *srvc_id, btgatt_char_id_t *char_id,
		bt_uuid_t *descr_id)
{
	char buf[MAX_UUID_STR_LEN];
	char srvc_id_buf[MAX_SRVC_ID_STR_LEN];
	char char_id_buf[MAX_CHAR_ID_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d srvc_id=%s char_id=%s, descr_id=%s\n",
				__func__, conn_id, status,
				btgatt_srvc_id_t2str(srvc_id, srvc_id_buf),
				btgatt_char_id_t2str(char_id, char_id_buf),
				gatt_uuid_t2str(descr_id, buf));

	if (status == 0)
		EXEC(if_gatt->client->get_descriptor, conn_id, srvc_id, char_id,
								descr_id);
}

/* GATT included service enumeration result callback */
static void gattc_get_included_service_cb(int conn_id, int status,
						btgatt_srvc_id_t *srvc_id,
						btgatt_srvc_id_t *incl_srvc_id)
{
	char srvc_id_buf[MAX_SRVC_ID_STR_LEN];
	char incl_srvc_id_buf[MAX_SRVC_ID_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d srvc_id=%s incl_srvc_id=%s)\n",
			__func__, conn_id, status,
			btgatt_srvc_id_t2str(srvc_id, srvc_id_buf),
			btgatt_srvc_id_t2str(incl_srvc_id, incl_srvc_id_buf));

	if (status == 0)
		EXEC(if_gatt->client->get_included_service, conn_id, srvc_id,
								incl_srvc_id);
}

/* Callback invoked in response to [de]register_for_notification */
static void gattc_register_for_notification_cb(int conn_id, int registered,
						int status,
						btgatt_srvc_id_t *srvc_id,
						btgatt_char_id_t *char_id)
{
	char srvc_id_buf[MAX_SRVC_ID_STR_LEN];
	char char_id_buf[MAX_CHAR_ID_STR_LEN];

	haltest_info("%s: conn_id=%d registered=%d status=%d srvc_id=%s char_id=%s\n",
				__func__, conn_id, registered, status,
				btgatt_srvc_id_t2str(srvc_id, srvc_id_buf),
				btgatt_char_id_t2str(char_id, char_id_buf));
}

/*
 * Remote device notification callback, invoked when a remote device sends
 * a notification or indication that a client has registered for.
 */
static void gattc_notify_cb(int conn_id, btgatt_notify_params_t *p_data)
{
	char buf[MAX_NOTIFY_PARAMS_STR_LEN];

	haltest_info("%s: conn_id=%d data=%s\n", __func__, conn_id,
				btgatt_notify_params_t2str(p_data, buf));
}

/* Reports result of a GATT read operation */
static void gattc_read_characteristic_cb(int conn_id, int status,
						btgatt_read_params_t *p_data)
{
	char buf[MAX_READ_PARAMS_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d data=%s\n", __func__, conn_id,
				status, btgatt_read_params_t2str(p_data, buf));
}

/* GATT write characteristic operation callback */
static void gattc_write_characteristic_cb(int conn_id, int status,
						btgatt_write_params_t *p_data)
{
	haltest_info("%s: conn_id=%d status=%d\n", __func__, conn_id, status);
}

/* GATT execute prepared write callback */
static void gattc_execute_write_cb(int conn_id, int status)
{
	haltest_info("%s: conn_id=%d status=%d\n", __func__, conn_id, status);
}

/* Callback invoked in response to read_descriptor */
static void gattc_read_descriptor_cb(int conn_id, int status,
						btgatt_read_params_t *p_data)
{
	char buf[MAX_READ_PARAMS_STR_LEN];

	haltest_info("%s: conn_id=%d status=%d data=%s\n", __func__, conn_id,
				status, btgatt_read_params_t2str(p_data, buf));
}

/* Callback invoked in response to write_descriptor */
static void gattc_write_descriptor_cb(int conn_id, int status,
						btgatt_write_params_t *p_data)
{
	haltest_info("%s: conn_id=%d status=%d\n", __func__, conn_id, status);
}

/* Callback triggered in response to read_remote_rssi */
static void gattc_read_remote_rssi_cb(int client_if, bt_bdaddr_t *bda, int rssi,
								int status)
{
	char buf[MAX_ADDR_STR_LEN];

	haltest_info("%s: client_if=%d bda=%s rssi=%d satus=%d\n", __func__,
			client_if, bt_bdaddr_t2str(bda, buf), rssi, status);
}

static const btgatt_client_callbacks_t btgatt_client_callbacks = {
	.register_client_cb = gattc_register_client_cb,
	.scan_result_cb = gattc_scan_result_cb,
	.open_cb = gattc_connect_cb,
	.close_cb = gattc_disconnect_cb,
	.search_complete_cb = gattc_search_complete_cb,
	.search_result_cb = gattc_search_result_cb,
	.get_characteristic_cb = gattc_get_characteristic_cb,
	.get_descriptor_cb = gattc_get_descriptor_cb,
	.get_included_service_cb = gattc_get_included_service_cb,
	.register_for_notification_cb = gattc_register_for_notification_cb,
	.notify_cb = gattc_notify_cb,
	.read_characteristic_cb = gattc_read_characteristic_cb,
	.write_characteristic_cb = gattc_write_characteristic_cb,
	.read_descriptor_cb = gattc_read_descriptor_cb,
	.write_descriptor_cb = gattc_write_descriptor_cb,
	.execute_write_cb = gattc_execute_write_cb,
	.read_remote_rssi_cb = gattc_read_remote_rssi_cb
};

/* BT-GATT Server callbacks */

/* Callback invoked in response to register_server */
static void gatts_register_server_cb(int status, int server_if,
							bt_uuid_t *app_uuid)
{
}

/*
 * Callback indicating that a remote device has connected
 * or been disconnected
 */
static void gatts_connection_cb(int conn_id, int server_if, int connected,
							bt_bdaddr_t *bda)
{
}

/* Callback invoked in response to create_service */
static void gatts_service_added_cb(int status, int server_if,
				btgatt_srvc_id_t *srvc_id, int srvc_handle)
{
}

/* Callback indicating that an included service has been added to a service */
static void gatts_included_service_added_cb(int status, int server_if,
							int srvc_handle,
							int incl_srvc_handle)
{
}

/* Callback invoked when a characteristic has been added to a service */
static void gatts_characteristic_added_cb(int status, int server_if,
								bt_uuid_t *uuid,
								int srvc_handle,
								int char_handle)
{
}

/* Callback invoked when a descriptor has been added to a characteristic */
static void gatts_descriptor_added_cb(int status, int server_if,
					bt_uuid_t *uuid, int srvc_handle,
							int descr_handle)
{
}

/* Callback invoked in response to start_service */
static void gatts_service_started_cb(int status, int server_if, int srvc_handle)
{
}

/* Callback invoked in response to stop_service */
static void gatts_service_stopped_cb(int status, int server_if, int srvc_handle)
{
}

/* Callback triggered when a service has been deleted */
static void gatts_service_deleted_cb(int status, int server_if, int srvc_handle)
{
}

/*
 * Callback invoked when a remote device has requested to read a characteristic
 * or descriptor. The application must respond by calling send_response
 */
static void gatts_request_read_cb(int conn_id, int trans_id, bt_bdaddr_t *bda,
						int attr_handle, int offset,
						bool is_long)
{
}

/*
 * Callback invoked when a remote device has requested to write to a
 * characteristic or descriptor.
 */
static void gatts_request_write_cb(int conn_id, int trans_id, bt_bdaddr_t *bda,
					int attr_handle, int offset, int length,
					bool need_rsp, bool is_prep,
					uint8_t *value)
{
}

/* Callback invoked when a previously prepared write is to be executed */
static void gatts_request_exec_write_cb(int conn_id, int trans_id,
					bt_bdaddr_t *bda, int exec_write)
{
}

/*
 * Callback triggered in response to send_response if the remote device
 * sends a confirmation.
 */
static void gatts_response_confirmation_cb(int status, int handle)
{
}

static const btgatt_server_callbacks_t btgatt_server_callbacks = {
	.register_server_cb = gatts_register_server_cb,
	.connection_cb = gatts_connection_cb,
	.service_added_cb = gatts_service_added_cb,
	.included_service_added_cb = gatts_included_service_added_cb,
	.characteristic_added_cb = gatts_characteristic_added_cb,
	.descriptor_added_cb = gatts_descriptor_added_cb,
	.service_started_cb = gatts_service_started_cb,
	.service_stopped_cb = gatts_service_stopped_cb,
	.service_deleted_cb = gatts_service_deleted_cb,
	.request_read_cb = gatts_request_read_cb,
	.request_write_cb = gatts_request_write_cb,
	.request_exec_write_cb = gatts_request_exec_write_cb,
	.response_confirmation_cb = gatts_response_confirmation_cb
};

static const btgatt_callbacks_t gatt_cbacks = {
	.size = sizeof(gatt_cbacks),
	.client = &btgatt_client_callbacks,
	.server = &btgatt_server_callbacks
};

/* gatt client methods */

/* init */

static void init_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_gatt);

	EXEC(if_gatt->init, &gatt_cbacks);
}

/* cleanup */

static void cleanup_p(int argc, const char **argv)
{
	RETURN_IF_NULL(if_gatt);

	EXECV(if_gatt->cleanup);

	if_gatt = NULL;
}

static struct method methods[] = {
	STD_METHOD(init),
	STD_METHOD(cleanup),
	END_METHOD
};

const struct interface gatt_if = {
	.name = "gatt",
	.methods = methods
};

/* register_client */

static void register_client_p(int argc, const char **argv)
{
	bt_uuid_t uuid;

	RETURN_IF_NULL(if_gatt);

	/* uuid */
	if (argc <= 2)
		gatt_str2bt_uuid_t("babe4bed", -1, &uuid);
	else
		gatt_str2bt_uuid_t(argv[2], -1, &uuid);

	EXEC(if_gatt->client->register_client, &uuid);
}

/* unregister_client */

static void unregister_client_p(int argc, const char **argv)
{
	int client_if;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);

	EXEC(if_gatt->client->unregister_client, client_if);
}

/* scan */

static void scan_p(int argc, const char **argv)
{
	int client_if;
	int start = 1;

	RETURN_IF_NULL(if_gatt);

	VERIFY_CLIENT_IF(2, client_if);

	/* start */
	if (argc >= 3)
		start = atoi(argv[3]);

	EXEC(if_gatt->client->scan, client_if, start);
}

/* connect */

static void connect_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;
	int is_direct = 1;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);

	/* is_direct */
	if (argc > 4)
		is_direct = atoi(argv[4]);

	EXEC(if_gatt->client->connect, client_if, &bd_addr, is_direct);
}

/* disconnect */

static void disconnect_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;
	int conn_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);
	VERIFY_CONN_ID(4, conn_id);

	EXEC(if_gatt->client->disconnect, client_if, &bd_addr, conn_id);
}

/* refresh */

static void refresh_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);

	EXEC(if_gatt->client->refresh, client_if, &bd_addr);
}

/* search_service */

static void search_service_p(int argc, const char **argv)
{
	int conn_id;
	bt_uuid_t filter_uuid;

	RETURN_IF_NULL(if_gatt);

	VERIFY_CONN_ID(2, conn_id);

	/* uuid */
	if (argc <= 3)
		memset(&filter_uuid, 0, sizeof(bt_uuid_t));
	else
		gatt_str2bt_uuid_t(argv[3], -1, &filter_uuid);

	EXEC(if_gatt->client->search_service, conn_id, &filter_uuid);
}

/* get_included_service */

static void get_included_service_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);

	EXEC(if_gatt->client->get_included_service, conn_id, &srvc_id, NULL);
}

/* get_characteristic */

static void get_characteristic_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);

	EXEC(if_gatt->client->get_characteristic, conn_id, &srvc_id, NULL);
}

/* get_descriptor */

static void get_descriptor_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);
	VERIFY_CHAR_ID(4, &char_id);

	EXEC(if_gatt->client->get_descriptor, conn_id, &srvc_id, &char_id,
									NULL);
}

/* read_characteristic */

static void read_characteristic_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;
	int auth_req = 0;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);
	VERIFY_CHAR_ID(4, &char_id);

	/* auth_req */
	if (argc > 5)
		auth_req = atoi(argv[5]);

	EXEC(if_gatt->client->read_characteristic, conn_id, &srvc_id, &char_id,
								auth_req);
}

/* write_characteristic */

static void write_characteristic_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;
	int write_type;
	int len;
	int auth_req = 0;
	uint8_t value[100];

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);
	VERIFY_CHAR_ID(4, &char_id);

	/* write type */
	if (argc <= 5) {
		haltest_error("No write type specified\n");
		return;
	}
	write_type = atoi(argv[5]);

	/* value */
	if (argc <= 6) {
		haltest_error("No value specified\n");
		return;
	}

	/* len in chars */
	len = strlen(argv[6]);
	scan_field(argv[6], len, value, sizeof(value));
	/* len in bytes converted from ascii chars */
	len = (len + 1) / 2;

	/* auth_req */
	if (argc > 7)
		auth_req = atoi(argv[7]);

	EXEC(if_gatt->client->write_characteristic, conn_id, &srvc_id, &char_id,
				write_type, len, auth_req, (char *) value);
}

/* read_descriptor */

static void read_descriptor_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;
	bt_uuid_t descr_id;
	int auth_req = 0;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);
	VERIFY_CHAR_ID(4, &char_id);
	VERIFY_UUID(5, &descr_id);

	/* auth_req */
	if (argc > 6)
		auth_req = atoi(argv[6]);

	EXEC(if_gatt->client->read_descriptor, conn_id, &srvc_id, &char_id,
							&descr_id, auth_req);
}

/* write_descriptor */

static void write_descriptor_p(int argc, const char **argv)
{
	int conn_id;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;
	bt_uuid_t descr_id;
	int write_type;
	int len;
	int auth_req = 0;
	uint8_t value[200] = {0};

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);
	VERIFY_SRVC_ID(3, &srvc_id);
	VERIFY_CHAR_ID(4, &char_id);
	VERIFY_UUID(5, &descr_id);

	/* write type */
	if (argc <= 6) {
		haltest_error("No write type specified\n");
		return;
	}
	write_type = atoi(argv[6]);

	/* value */
	if (argc <= 7) {
		haltest_error("No value specified\n");
		return;
	}

	/* len in chars */
	len = strlen(argv[7]);
	scan_field(argv[7], len, value, sizeof(value));
	/* len in bytes converted from ascii chars */
	len = (len + 1) / 2;

	/* auth_req */
	if (argc > 8)
		auth_req = atoi(argv[8]);

	EXEC(if_gatt->client->write_descriptor, conn_id, &srvc_id, &char_id,
			&descr_id, write_type, len, auth_req, (char *) value);
}

/* execute_write */

static void execute_write_p(int argc, const char **argv)
{
	int conn_id;
	int execute;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CONN_ID(2, conn_id);

	/* execute */
	if (argc <= 3) {
		haltest_error("No execute specified\n");
		return;
	}
	execute = atoi(argv[3]);

	EXEC(if_gatt->client->execute_write, conn_id, execute);
}

/* register_for_notification */

static void register_for_notification_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);
	VERIFY_SRVC_ID(4, &srvc_id);
	VERIFY_CHAR_ID(5, &char_id);

	EXEC(if_gatt->client->register_for_notification, client_if, &bd_addr,
							&srvc_id, &char_id);
}

/* deregister_for_notification */

static void deregister_for_notification_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;
	btgatt_srvc_id_t srvc_id;
	btgatt_char_id_t char_id;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);
	VERIFY_SRVC_ID(4, &srvc_id);
	VERIFY_CHAR_ID(5, &char_id);

	EXEC(if_gatt->client->deregister_for_notification, client_if, &bd_addr,
							&srvc_id, &char_id);
}

/* read_remote_rssi */

static void read_remote_rssi_p(int argc, const char **argv)
{
	int client_if;
	bt_bdaddr_t bd_addr;

	RETURN_IF_NULL(if_gatt);
	VERIFY_CLIENT_IF(2, client_if);
	VERIFY_ADDR_ARG(3, &bd_addr);

	EXEC(if_gatt->client->read_remote_rssi, client_if, &bd_addr);
}

/* get_device_type */

static void get_device_type_p(int argc, const char **argv)
{
	bt_bdaddr_t bd_addr;
	int dev_type;

	RETURN_IF_NULL(if_gatt);
	VERIFY_ADDR_ARG(2, &bd_addr);

	dev_type = if_gatt->client->get_device_type(&bd_addr);
	haltest_info("%s: %d\n", "get_device_type", dev_type);
}

/* test_command */

static void test_command_p(int argc, const char **argv)
{
	int command;
	int i;
	bt_bdaddr_t bd_addr;
	bt_uuid_t uuid;
	btgatt_test_params_t params = {
		.bda1 = &bd_addr,
		.uuid1 = &uuid
	};
	uint16_t *u = &params.u1;

	RETURN_IF_NULL(if_gatt);

	/* command */
	if (argc <= 2) {
		haltest_error("No command specified\n");
		return;
	}
	command = atoi(argv[2]);

	VERIFY_ADDR_ARG(3, &bd_addr);
	VERIFY_UUID(4, &uuid);

	for (i = 5; i < argc; i++)
		*u++ = atoi(argv[i]);

	EXEC(if_gatt->client->test_command, command, &params);
}

static struct method client_methods[] = {
	STD_METHODH(register_client, "[<uuid>]"),
	STD_METHODH(unregister_client, "<client_if>"),
	STD_METHODH(scan, "<client_if> [1|0]"),
	STD_METHODH(connect, "<client_if> <addr> [<is_direct>]"),
	STD_METHODH(disconnect, "<client_if> <addr> <conn_id>"),
	STD_METHODH(refresh, "<client_if> <addr>"),
	STD_METHODH(search_service, "<conn_id> [<uuid>]"),
	STD_METHODH(get_included_service, "<conn_id> <srvc_id>"),
	STD_METHODH(get_characteristic, "<conn_id> <srvc_id>"),
	STD_METHODH(get_descriptor, "<conn_id> <srvc_id> <char_id>"),
	STD_METHODH(read_characteristic,
			"<conn_id> <srvc_id> <char_id> [<auth_req>]"),
	STD_METHODH(write_characteristic,
			"<conn_id> <srvc_id> <char_id> <write_type> <hex_value> [<auth_req>]"),
	STD_METHODH(read_descriptor,
			"<conn_id> <srvc_id> <char_id> <descr_id> [<auth_req>]"),
	STD_METHODH(write_descriptor,
			"<conn_id> <srvc_id> <char_id> <descr_id> <write_type> <hex_value> [<auth_req>]"),
	STD_METHODH(execute_write, "<conn_id> <execute>"),
	STD_METHODH(register_for_notification,
			"<client_if> <addr> <srvc_id> <char_id>"),
	STD_METHODH(deregister_for_notification,
			"<client_if> <addr> <srvc_id> <char_id>"),
	STD_METHODH(read_remote_rssi, "<client_if> <addr>"),
	STD_METHODH(get_device_type, "<addr>"),
	STD_METHODH(test_command,
			"<cmd> <addr> <uuid> [u1] [u2] [u3] [u4] [u5]"),
	END_METHOD
};

const struct interface gatt_client_if = {
	.name = "gattc",
	.methods = client_methods
};

/* gatt server methods */

/* register_server */

static void gatts_register_server_p(int argc, const char *argv[])
{
}

/* unregister_server */

static void gatts_unregister_server_p(int argc, const char *argv[])
{
}

/* connect */

static void gatts_connect_p(int argc, const char *argv[])
{
}

/* disconnect */

static void gatts_disconnect_p(int argc, const char *argv[])
{
}

/* add_service */

static void gatts_add_service_p(int argc, const char *argv[])
{
}

/* add_included_service */

static void gatts_add_included_service_p(int argc, const char *argv[])
{
}

/* add_characteristic */

static void gatts_add_characteristic_p(int argc, const char *argv[])
{
}

/* add_descriptor */

static void gatts_add_descriptor_p(int argc, const char *argv[])
{
}

/* start_service */

static void gatts_start_service_p(int argc, const char *argv[])
{
}

/* stop_service */

static void gatts_stop_service_p(int argc, const char *argv[])
{
}

/* delete_service */

static void gatts_delete_service_p(int argc, const char *argv[])
{
}

/* send_indication */

static void gatts_send_indication_p(int argc, const char *argv[])
{
}

/* send_response */

static void gatts_send_response_p(int argc, const char *argv[])
{
}

#define GATTS_METHODH(n, h) METHOD(#n, gatts_##n##_p, NULL, h)
#define GATTS_METHODCH(n, h) METHOD(#n, gatts_##n##_p, gatts_##n##_c, h)

static struct method server_methods[] = {
	GATTS_METHODH(register_server, "[<uuid>]"),
	GATTS_METHODH(unregister_server, "<server_if>"),
	GATTS_METHODH(connect, "<server_if> <addr> [<is_direct>]"),
	GATTS_METHODH(disconnect, "<server_if> <addr> <conn_id>"),
	GATTS_METHODH(add_service, "<server_if> <srvc_id> <num_handles>"),
	GATTS_METHODH(add_included_service,
			"<server_if> <service_handle> <included_handle>"),
	GATTS_METHODH(add_characteristic,
		"<server_if> <service_handle> <uuid> <properites> <permissions>"),
	GATTS_METHODH(add_descriptor, "<server_if> <uuid> <permissions>"),
	GATTS_METHODH(start_service,
				"<server_if> <service_handle> <transport>"),
	GATTS_METHODH(stop_service, "<server_if> <service_handle>"),
	GATTS_METHODH(delete_service, "<server_if> <service_handle>"),
	GATTS_METHODH(send_indication,
			"<server_if> <attr_handle> <conn_id> <confirm> [<data>]"),
	GATTS_METHODH(send_response, "<conn_id> <trans_id> <status>"),
	END_METHOD
};

const struct interface gatt_server_if = {
	.name = "gatts",
	.methods = server_methods
};
