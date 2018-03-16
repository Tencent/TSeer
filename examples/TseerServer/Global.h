#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <string>

#include "servant/Communicator.h"

std::string g_key = "webadmin";

std::string g_url = "http://127.0.0.1:9904/v1/interface";

std::string g_srvgrp = "apitest222";

std::string g_user = "";
std::string g_idcname = "idctest";
tars::Communicator g_com;

int g_ttime = 0;

int g_iloadInterval = 0;

#endif