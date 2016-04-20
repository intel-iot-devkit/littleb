#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <systemd/sd-bus.h>

#define MAX_LEN 256
#define MAX_OBJECTS 256

typedef struct bl_device {
	const char* object_path;
	const char* address;
	const char* name;
}bl_device;

sd_bus_error error = SD_BUS_ERROR_NULL;
sd_bus_message *m = NULL;
sd_bus *bus = NULL;
bl_device** devices = NULL;
int devices_size = 0;

int finish() {
	int r;
	sd_bus_error_free(&error);
	sd_bus_message_unref(m);
	sd_bus_unref(bus);
	if (devices) {
		free(devices);
	}

	return r < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}

const char* convert_address_path(const char* address)
{
	int i;
	char* new_address = malloc(strlen(address) * sizeof(char));
	if (new_address == NULL) {
		fprintf(stderr, "Error allocating memory for new_address\n");
		return NULL;
	}
	strncpy(new_address, address, strlen(address));
	if (new_address == NULL) {
		fprintf(stderr, "Error copying address to new_addresss\n");
		return NULL;
	}

	for (i = 0; i < strlen(new_address); i++) {
		if (new_address[i] == '_')
			new_address[i] = ':';
	}
	return (const char*) new_address;
}

int open_system_bus() {
	int r;
	/* Connect to the system bus */
	r = sd_bus_open_system(&bus);
	if (r < 0) {
		fprintf(stderr, "Failed to connect to system bus: %s\n", strerror(-r));
		return finish();
	}
	return r;
}

bool is_bus_connected() {
	if (bus == NULL) {
		return false;
	} else {
		return (sd_bus_is_open(bus)) ? true : false;
	}
}

const char* get_device_name(const char* device_path)
{
	int r;
	char* name;

	r = sd_bus_get_property_string(
								bus,
								"org.bluez",
								device_path,
								"org.bluez.Device1",
								"Name",
								&error,
								&name);
	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s on %s\n", error.message, device_path);
		return NULL;
	}
	printf("Method Called: %s\n", "sd_bus_get_property_string");

	return (const char *) name;
}

int add_new_device(const char* device_path)
{
	int current_index = devices_size;
	if (devices_size == 0 || devices == NULL)
	{
		devices = malloc(sizeof(bl_device*));
		devices_size++;
	}
	else {
		devices_size++;
		devices = realloc(devices, (devices_size) * sizeof(bl_device*));
		if (devices == NULL) {
			fprintf(stderr, "Error reallocating memory for devices\n");
			return EXIT_FAILURE;
		}
	}

	bl_device* new_device = malloc(sizeof(bl_device));

	const char* point = strstr(device_path, "dev_");
	if (point != NULL) {
		new_device->object_path = device_path;
		const char* name = get_device_name(device_path);
		if (name == NULL) {
			fprintf(stderr, "Error couldn't find device name\n");
			new_device->name = "null";
		}
		else {
			new_device->name = name;
		}
		new_device->address = convert_address_path(point+4);
		devices[current_index] = new_device;
	}

	return EXIT_SUCCESS;
}

int get_root_objects(const char** objects) {
	int r;
	const char *object_path;
	int i = 0;

	if (!is_bus_connected()) {
		fprintf(stderr, "Bus is not opened\n");
	}

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/", 									/* object path */
							"org.freedesktop.DBus.ObjectManager", 	/* interface name */
							"GetManagedObjects", 					/* method name */
							&error, 								/* object to return error in */
							&m, 									/* return message on success */
							NULL); 									/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "GetManagedObjects");

	/* Parse the response message */
	//"a{oa{sa{sv}}}"
	r = sd_bus_message_enter_container(m, 'a', "{oa{sa{sv}}}");
	if (r < 0) {
		fprintf(stderr, "sd_bus_message_enter_container {oa{sa{sv}}}: %s\n",
				strerror(-r));
		return finish();
	}

	while ((r = sd_bus_message_enter_container(m, 'e', "oa{sa{sv}}")) > 0) {
		r = sd_bus_message_read_basic(m, 'o', &object_path);
		if (r < 0) {
			fprintf(stderr, "sd_bus_message_read_basic: %s\n", strerror(-r));
			return finish();
		} else {
			char* restrict new_object_path = malloc(strlen(object_path) + 1);
			if (new_object_path == NULL) {
				fprintf(stderr, "Error allocating memory for object name\n");
				return finish();
			}
			objects[i] = strcpy(new_object_path, object_path);
			i++;
		}

		r = sd_bus_message_skip(m, "a{sa{sv}}");
		if (r < 0) {
			fprintf(stderr, "sd_bus_message_skip: %s\n", strerror(-r));
			return finish();
		}

		r = sd_bus_message_exit_container(m);
		if (r < 0) {
			fprintf(stderr, "sd_bus_message_exit_container oa{sa{sv}}: %s\n",
					strerror(-r));
			return finish();
		}
	}

	if (r < 0) {
		fprintf(stderr, "sd_bus_message_enter_container oa{sa{sv}}: %s\n",
				strerror(-r));
		return finish();
	}

	r = sd_bus_message_exit_container(m);
	if (r < 0) {
		fprintf(stderr, "sd_bus_message_exit_container {oa{sa{sv}}}: %s\n",
				strerror(-r));
		return finish();
	}

	return finish();
}

int scan_devices(int seconds)
{
	int r;

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0", 						/* object path */
							"org.bluez.Adapter1", 						/* interface name */
							"StartDiscovery", 							/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "StartDiscovery");

	sleep(seconds);

	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0", 						/* object path */
							"org.bluez.Adapter1", 						/* interface name */
							"StopDiscovery", 							/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "StopDiscovery");

	return finish();
}

int list_devices(int seconds)
{
	const char** objects;
	const char* point;
	int i, r;

	if (devices != NULL) {
		free(devices);
	}

	objects = malloc(MAX_OBJECTS * sizeof(const char *));
	if (objects == NULL) {
		fprintf(stderr, "Error allocating memory for objects array\n");
		return EXIT_FAILURE;
	}

	scan_devices(seconds);

	get_root_objects(objects);

	//if (r < 0) {
	//	fprintf(stderr, "Error getting root objects\n");
	//	free(objects);
	//	return -1;
	//}

	while(objects[i] != NULL) {
		point = strstr(objects[i], "dev_");
		if (point != NULL) {
			add_new_device(objects[i]);
		}
		i++;
	}

	free(objects);
	return 0;
}

int connect_device(const char * address) {
	int r;

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0/dev_98_4F_EE_0F_42_B4", 	/* object path */
							"org.bluez.Device1", 						/* interface name */
							"Connect", 									/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "Connect");

	return finish();
}

int disconnect_device(const char * address) {
	int r;

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0/dev_98_4F_EE_0F_42_B4", 	/* object path */
							"org.bluez.Device1", 						/* interface name */
							"Disconnect", 								/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "Disconnect");

	return finish();
}

int pair_device(const char * address) {
	int r;

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0/dev_98_4F_EE_0F_42_B4", 	/* object path */
							"org.bluez.Device1", 						/* interface name */
							"Pair", 									/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "Pair");

	return finish();
}

int unpair_device(const char * address) {
	int r;

	/* Issue the method call and store the respons message in m */
	r = sd_bus_call_method(	bus,
							"org.bluez", 							/* service to contact */
							"/org/bluez/hci0/dev_98_4F_EE_0F_42_B4", 	/* object path */
							"org.bluez.Device1", 						/* interface name */
							"CancelPairing", 							/* method name */
							&error, 									/* object to return error in */
							&m, 										/* return message on success */
							NULL); 										/* input signature */

	if (r < 0) {
		fprintf(stderr, "Failed to issue method call: %s\n", error.message);
		return finish();
	}

	printf("Method Called: %s\n", "CancelPairing");

	return finish();
}

int write_to_char(const char* address, const char* value) {

	return finish();
}

int main(int argc, char *argv[]) {
	open_system_bus();
	list_devices(10);
	int i;
	for (i = 0; i < devices_size; i++) {
		printf("%s\t%s\n", devices[i]->address, devices[i]->name);
	}
	//connect_device("");
	//pair_device("");
	//unpair_device("");
	//disconnect_device("");
	return 0;
}
