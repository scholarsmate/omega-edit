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

import {client} from './settings'
import {ChangeKind, ChangeRequest, CountKind, CountRequest, ObjectId} from '../../omega_edit_pb'

export function ins(session_id: string, offset: number, data: string | Uint8Array): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        let request = new ChangeRequest().setSessionId(session_id).setOffset(offset)
        request.setKind(ChangeKind.CHANGE_INSERT)
        request.setData((typeof data == 'string') ? new TextEncoder().encode(data) : data)
        client.submitChange(request, (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('ins error: ' + err.message)
            }

            return resolve(r.getSerial())
        })
    })
}

export function del(session_id: string, offset: number, len: number): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        let request = new ChangeRequest().setSessionId(session_id).setOffset(offset)
        request.setKind(ChangeKind.CHANGE_DELETE)
        request.setLength(len)
        client.submitChange(request, (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('del error: ' + err.message)
            }

            return resolve(r.getSerial())
        })
    })
}

export function ovr(session_id: string, offset: number, data: string | Uint8Array): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        let request = new ChangeRequest().setSessionId(session_id).setOffset(offset)
        request.setKind(ChangeKind.CHANGE_OVERWRITE)
        request.setData((typeof data == 'string') ? new TextEncoder().encode(data) : data)
        client.submitChange(request, (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('ovr error: ' + err.message)
            }

            return resolve(r.getSerial())
        })
    })
}

export function undo(session_id: string): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        client.undoLastChange(new ObjectId().setId(session_id), (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('undo error: ' + err.message)
            }
            return resolve(r.getSerial())
        })
    })
}

export function redo(session_id: string): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        client.redoLastUndo(new ObjectId().setId(session_id), (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('redo error: ' + err.message)
            }
            return resolve(r.getSerial())
        })
    })
}

export function getChangeCount(sesssion_id: string): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        client.getCount(new CountRequest().setSessionId(sesssion_id).setKind(CountKind.COUNT_CHANGES), (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('redo error: ' + err.message)
            }
            return resolve(r.getCount())
        })
    })
}

export function getUndoCount(sesssion_id: string): Promise<number> {
    return new Promise<number>((resolve, reject) => {
        client.getCount(new CountRequest().setSessionId(sesssion_id).setKind(CountKind.COUNT_UNDOS), (err, r) => {
            if (err) {
                console.log(err.message)
                return reject('redo error: ' + err.message)
            }
            return resolve(r.getCount())
        })
    })
}
