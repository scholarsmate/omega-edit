/*
 * Copyright (c) 2021 Concurrent Technologies Corporation.
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { EditorClient } from './omega_edit_grpc_pb'
import * as grpc from '@grpc/grpc-js'

declare module 'omega-edit/settings' {
  export const ALL_EVENTS
  export function initClient(host: string, port: string)
  export function getClient(host?: string, port?: string): EditorClient
  export function waitForReady(
    client: EditorClient,
    deadline: grpc.Deadline
  ): Promise<boolean>
}
