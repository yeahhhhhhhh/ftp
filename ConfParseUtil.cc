#include <iostream>
#include <string.h>
#include <ctype.h>
#include "ConfParseUtil.h"
#include "StringUtil.h"
#include "Common.h"

using namespace std;

int ConfParseUtil::pasvEnable = -1;
int ConfParseUtil::portEnable = -1;
unsigned int ConfParseUtil::listenPort = 0;
unsigned int ConfParseUtil::maxClients = 0;
unsigned int ConfParseUtil::maxPerIp = 0;
unsigned int ConfParseUtil::connectTimeout = 0;
unsigned int ConfParseUtil::idleSessionTime = 0;
unsigned int ConfParseUtil::dataConnectionTimeout = 0;
unsigned int ConfParseUtil::localUmask = 0;
unsigned int ConfParseUtil::uploadMaxRate = 0;
unsigned int ConfParseUtil::downloadMaxRate = 0;

ConfParseUtil::ConfParseUtil(){
	ConfBoolSetting confBoolSetting[] = {{"pasv_enable",&pasvEnable},
										 {"port_enable",&portEnable},
										 {NULL,NULL}};
	ConfUintSetting confUintSetting[] = {{"listen_port",&listenPort},
										 {"max_clients",&maxClients},
										 {"max_per_ip",&maxPerIp},
										 {"connect_timeout",&connectTimeout},
										 {"idle_session_time",&idleSessionTime},
										 {"data_connection_timeout",&dataConnectionTimeout},
										 {"local_umask",&localUmask},
										 {"upload_max_rate",&uploadMaxRate},
										 {"download_max_rate",&downloadMaxRate},
										 {NULL,NULL}};
	confBoolSetting_ = confBoolSetting;
	confUintSetting_ = confUintSetting;
	
	FILE *fp = fopen(MINIFTP_CONF, "r");
	if(fp == NULL)
		ERR_EXIT("fopen");
	char settingLine[1024] = {0};
	while(fgets(settingLine, sizeof(settingLine), fp) != NULL){
		if(strlen(settingLine) == 0 || settingLine[0] == '#')
			continue;
		StringUtil::trimCtlf(settingLine);
		// load data
		parseConfLoadSetting(settingLine);
		memset(settingLine, 0, sizeof(settingLine));
	}
	fclose(fp);
	
}

ConfParseUtil::~ConfParseUtil(){}

void ConfParseUtil::parseConfLoadSetting(const char* settingLine){
	ConfBoolSetting* confBoolSetting = confBoolSetting_;
	ConfUintSetting* confUintSetting = confUintSetting_;
	// remove left space
	while(isspace(*settingLine))
		settingLine++;
	char key[128] = {0};
	char value[128] = {0};
	StringUtil::split(settingLine, key, value, '=');
	if(strlen(value) == 0){
		cout << "missing value in config file for " << key << endl;
		exit(EXIT_FAILURE);
	}
	while(confBoolSetting -> key != NULL){
		if(strcmp(key, confBoolSetting -> key) == 0){
			if(strcmp(value, "YES") || strcmp(value, "1") || strcmp(value, "TRUE")){
				*(confBoolSetting -> value) = 1;
				return;
			}
		}
		confBoolSetting++;
	}
	while(confUintSetting -> key != NULL){
		if(strcmp(key, confUintSetting -> key) == 0){
			*(confUintSetting -> value) = atoi(value);
			return;
		}
		confUintSetting++;
	}
}

