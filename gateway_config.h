/*
 * Copyright 2020 Cypress Semiconductor Corporation
 * SPDX-License-Identifier: Apache-2.0
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
#pragma once

// Note : currently user can enable only one transport
#define APP_CONFIG_AWS_CLOUD 1
// #define APP_CONFIG_HTTP_SERVER 1

#ifdef APP_CONFIG_AWS_CLOUD
#include "gateway_aws_config.h"
#endif

#ifdef APP_CONFIG_HTTP_SERVER
/** Add HTTP-server related configuration header file here, if any */
#endif

