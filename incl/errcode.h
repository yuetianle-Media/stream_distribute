#pragma once

#define E_OK 0
#pragma region NETWORK
#define E_ERROR -1
#define E_CONN_ERROR -10
#define E_CONN_TIMEOUT -11
#define E_REMOTE_ADDR_EMPTY -12
#define E_SEND_ERROR -16
#define E_MESG_END -100
#pragma endregion NETWORK

#pragma region File
#define E_FILE_EMPTY -1000
#define E_FILE_ERROR -1001
#define E_FILE_BAD_FORMAT -1002
#pragma endregion File

#pragma region Buffer
#define E_BUFFER_EMPTY -2000
#define E_BUFFER_FULL -2001
#define E_BUFFER_LESS -2002
#pragma endregion Buffer

#pragma region DATA
#define E_DATA_EMPTY -3000
#pragma endregion DATA

#pragma region BASE 
#define E_PARAM_ERROR -4000
#pragma endregion BASE
