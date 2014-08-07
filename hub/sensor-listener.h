#ifndef SENSOR_LISTENER_H
#define SENSOR_LISTENER_H

#include <mysql.h>
#include <libconfig.h++>
#include <RF24.h>

#define SKIPJACK_KEY_SIZE 10
#define SIGNATURE_KEY_SIZE 4
#define SIGNATURE_SIZE 5

using namespace libconfig;

const char version[]           = "0.1";
const char progname[]          = "sensor-listener";

Config cfg;
const char * zabbixserver;
const char * zabbixclient;
int zabbixport;

const char * mysqlserver;
const char * mysqldb;
const char * mysqluser;
const char * mysqlpass;

int sensorhubid;

//nRF24L01+, raspberry pi vs beaglebone black macros
#if defined __ARM_ARCH_6__
    RF24 radio("/dev/spidev0.0",8000000 , 25);
#elif defined __ARM_ARCH_7A__
    RF24 radio(115, 117);
#else // build on non-arm with bogus initializer
    RF24 radio(115, 117);
#endif
uint64_t pipes[6];

// this struct should always be a multiple of 64 bits so we can easily encrypt it (skipjack 64bit blocks)
struct __attribute__((packed))
sensordata {
    int8_t   addr;
    bool     proximity; // is reed switch open?
    int8_t   placeholder2; // future var
    int16_t  tempc; // temp in centicelsius
    int16_t  humidity; // humidity in basis points (percent of percent)
    uint16_t pressuredp; // pressure in decapascals
    uint16_t millivolts; // battery voltage millivolts
    uint8_t  signature[SIGNATURE_SIZE]; //truncated md5 of sensor data
};

// struct message = 28 bytes. Want to keep this <= 32 bytes so that it will fit in one radio message
struct message {
    int8_t addr;
    bool encrypted;
    char enckey[SKIPJACK_KEY_SIZE];
    char sigkey[SIGNATURE_KEY_SIZE];
    sensordata s;
};

//prototypes
void radio_init();
void get_args(int argc, char *argv[]);
void usage();
bool zabbix_send(const char * key, const char * value);
bool publish_zabbix(message *m);
int insert_neutrino_data(message *m);
float ctof(int16_t c);
bool decrypt_sensordata(message *m);
bool check_sensordata_signature(message *m);
bool encryption_key_is_empty(char * key);
bool sensor_is_known(int sensorid);
bool listener_should_discover();
unsigned int get_sensor_id(MYSQL *conn, int sensorid, int sensorhubid);
MYSQL_ROW sql_select_row(char * sql);


#endif