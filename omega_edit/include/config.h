/*
* Copyright 2021 Concurrent Technologies Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef OMEGA_EDIT_CONFIG_H
#define OMEGA_EDIT_CONFIG_H

/***********************************************************************************************************************
 * CONFIGURATION
 **********************************************************************************************************************/

// Define to enable debugging
//#define DEBUG

// Default maximum viewport capacity
#ifndef DEFAULT_VIEWPORT_MAX_CAPACITY
#define DEFAULT_VIEWPORT_MAX_CAPACITY (1024 * 1024)
#endif//DEFAULT_VIEWPORT_MAX_CAPACITY

#ifndef NEEDLE_LENGTH_LIMIT
#define NEEDLE_LENGTH_LIMIT (DEFAULT_VIEWPORT_MAX_CAPACITY / 2)
#endif//NEEDLE_LENGTH_LIMIT

// Define the byte type to be used across the project
#ifndef BYTE_T
#define BYTE_T unsigned char
#endif//BYTE_T

#endif//OMEGA_EDIT_CONFIG_H
