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

import {
  ByteFrequencyProfileRequest,
  CreateSessionRequest,
  ObjectId,
  SaveSessionRequest,
  SearchRequest,
  SegmentRequest,
} from './omega_edit_pb'
import { Empty } from 'google-protobuf/google/protobuf/empty_pb'
import { getClient } from './settings'
import { replaceOptimized } from './change'

/**
 * Create a file editing session from a file path
 * @param file_path file path, will be opened for read, to create an editing session with, or undefined if starting from
 * scratch
 * @param session_id_desired if defined, the session ID to assign to this session, if undefined a unique session ID will
 * be generated by the server
 * @return session ID, on success
 */
export function createSession(
  file_path: string | undefined,
  session_id_desired: string | undefined
): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    let request = new CreateSessionRequest()
    if (session_id_desired !== undefined && session_id_desired.length > 0)
      request.setSessionIdDesired(session_id_desired)
    if (file_path !== undefined && file_path.length > 0)
      request.setFilePath(file_path)
    getClient().createSession(request, (err, r) => {
      if (err) {
        console.log(err.message)
        return reject('createSession error: ' + err.message)
      }
      return resolve(r.getSessionId())
    })
  })
}

/**
 * Destroy the given session and all associated objects (changes, and viewports)
 * @param session_id session to destroy
 * @return session ID that was destroyed, on success
 */
export function destroySession(session_id: string): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    getClient().destroySession(new ObjectId().setId(session_id), (err, r) => {
      if (err) {
        console.log(err.message)
        return reject('destroySession error: ' + err.message)
      }
      return resolve(r.getId())
    })
  })
}

/**
 * Save the given session (the edited file) to the given file path.  If the save file already exists, it can be
 * overwritten if overwrite is true.  If the file exists and overwrite is false, a new unique file name will be used as
 * determined by server.  If the file being edited is overwritten, the affected editing session will be reset.
 * @param session_id session to save
 * @param file_path file path to save to
 * @param overwrite set to true if overwriting an existing file is okay, and false otherwise
 * @return name of the saved file, on success
 */
export function saveSession(
  session_id: string,
  file_path: string,
  overwrite: boolean
): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    getClient().saveSession(
      new SaveSessionRequest()
        .setSessionId(session_id)
        .setFilePath(file_path)
        .setAllowOverwrite(overwrite),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('saveSession error: ' + err.message)
        }
        return resolve(r.getFilePath())
      }
    )
  })
}

/**
 * Computed file size in bytes for a given session
 * @param session_id session to get the computed file size from
 * @return computed file size in bytes, on success
 */
export function getComputedFileSize(session_id: string): Promise<number> {
  return new Promise<number>((resolve, reject) => {
    getClient().getComputedFileSize(
      new ObjectId().setId(session_id),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('getComputedFileSize error: ' + err.message)
        }
        return resolve(r.getComputedFileSize())
      }
    )
  })
}

/**
 * Pause data changes to the session
 * @param session_id session to pause changes to
 * @return session ID that has its changes paused, on success
 */
export function pauseSessionChanges(session_id: string): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    getClient().pauseSessionChanges(
      new ObjectId().setId(session_id),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('pauseSessionChanges error: ' + err.message)
        }
        return resolve(r.getId())
      }
    )
  })
}

/**
 * Resume data changes on the previously paused session
 * @param session_id session to resume changes on
 * @return session ID that has its changes resumed, on success
 */
export function resumeSessionChanges(session_id: string): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    getClient().resumeSessionChanges(
      new ObjectId().setId(session_id),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('resumeSessionChanges error: ' + err.message)
        }
        return resolve(r.getId())
      }
    )
  })
}

/**
 * Unsubscribe to session events
 * @param session_id session to unsubscribe
 * @return session ID that was unsubscribed, on success
 */
export function unsubscribeSession(session_id: string): Promise<string> {
  return new Promise<string>((resolve, reject) => {
    getClient().unsubscribeToSessionEvents(
      new ObjectId().setId(session_id),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('unsubscribeSession error: ' + err.message)
        }
        return resolve(r.getId())
      }
    )
  })
}

/**
 * Given a session and offset, return a copy of that data segment
 * @param session_id session to copy a segment of data from
 * @param offset session offset to begin copying data from
 * @param length number of bytes to copy
 * @return copy of the desired segment of data, on success
 */
export function getSegment(
  session_id: string,
  offset: number,
  length: number
): Promise<Uint8Array> {
  return new Promise<Uint8Array>((resolve, reject) => {
    getClient().getSegment(
      new SegmentRequest()
        .setSessionId(session_id)
        .setOffset(offset)
        .setLength(length),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('getSegment error: ' + err.message)
        }
        return resolve(r.getData_asU8())
      }
    )
  })
}

/**
 * Gets the number of active editing sessions on the server
 * @return number of active sessions on the server, on success
 */
export function getSessionCount(): Promise<number> {
  return new Promise<number>((resolve, reject) => {
    getClient().getSessionCount(new Empty(), (err, r) => {
      if (err) {
        console.log(err.message)
        return reject('getSessionCount error: ' + err.message)
      }
      return resolve(r.getCount())
    })
  })
}

/**
 * Notify changed viewports in the given session with a VIEWPORT_EVT_CHANGES event
 * @param session_id session to notify viewports with changes
 * @return number of viewports that were notified
 */
export function notifyChangedViewports(session_id: string): Promise<number> {
  return new Promise<number>((resolve, reject) => {
    getClient().notifyChangedViewports(
      new ObjectId().setId(session_id),
      (err, r) => {
        if (err) {
          console.log(err.message)
          return reject('notifyChangedViewports error: ' + err.message)
        }
        return resolve(r.getResponse())
      }
    )
  })
}

/**
 * Given a session, offset and length, populate a byte frequency profile
 * @param session_id session to profile
 * @param offset where in the session to begin profiling
 * @param length number of bytes from the offset to stop profiling (if 0, it will profile to the end of the session)
 * @return array of size 256 (for the 8-bit bytes) with the values being the byte frequency in the given range, on
 * success
 */
export function profileSession(
  session_id: string,
  offset: number | undefined,
  length: number | undefined
): Promise<number[]> {
  return new Promise<number[]>((resolve, reject) => {
    let request = new ByteFrequencyProfileRequest().setSessionId(session_id)
    if (offset !== undefined && offset >= 0) request.setOffset(offset)
    if (length !== undefined && length > 0) request.setLength(length)
    getClient().getByteFrequencyProfile(request, (err, r) => {
      if (err) {
        console.log(err.message)
        return reject('searchSession error: ' + err.message)
      }
      return resolve(r.getFrequencyList())
    })
  })
}

/**
 * Given a computed profile, return the total number of bytes in the 7-bit ASCII range
 * @param profile computed profile from profileSession
 * @return total number of ASCII bytes found in the profile
 */
export function numAscii(profile: number[]): number {
  return profile.slice(0, 128).reduce((accumulator, current) => {
    return accumulator + current
  }, 0)
}

/**
 * Search a segment in a session for a given pattern and return an array of offsets where the pattern was found
 * @param session_id session to find the pattern in
 * @param pattern pattern to find
 * @param is_case_insensitive false for case-sensitive matching and true for case-insensitive matching
 * @param offset start searching at this offset within the session, or at the start of the session if undefined
 * @param length search from the starting offset within the session up to this many bytes, if set to zero or undefined,
 * it will search to the end of the session
 * @param limit if defined, limits the number of matches found to this amount
 * @return array of offsets where the pattern was found
 */
export function searchSession(
  session_id: string,
  pattern: string | Uint8Array,
  is_case_insensitive: boolean | undefined,
  offset: number | undefined,
  length: number | undefined,
  limit: number | undefined
): Promise<number[]> {
  return new Promise<number[]>((resolve, reject) => {
    let request = new SearchRequest()
      .setSessionId(session_id)
      .setPattern(typeof pattern === 'string' ? Buffer.from(pattern) : pattern)
      .setIsCaseInsensitive(is_case_insensitive ?? false)
    if (offset !== undefined && offset >= 0) request.setOffset(offset)
    if (length !== undefined && length > 0) request.setLength(length)
    if (limit !== undefined && limit > 0) request.setLimit(limit)
    getClient().searchSession(request, (err, r) => {
      if (err) {
        console.log(err.message)
        return reject('searchSession error: ' + err.message)
      }
      return resolve(r.getMatchOffsetList())
    })
  })
}

/**
 * Replace all found patterns in a segment in a session with the given replacement and return the number of replacements
 * done
 * @param session_id session to replace patterns in
 * @param pattern pattern to replace
 * @param replacement replacement
 * @param is_case_insensitive false for case-sensitive matching and true for case-insensitive matching
 * @param offset start searching at this offset within the session, or at the start of the session if undefined
 * @param length search from the starting offset within the session up to this many bytes, if set to zero or undefined,
 * it will search to the end of the session
 * @param limit if defined, limits the number of matches found to this amount
 * @return number of replacements done
 * @remarks highly recommend pausing all viewport events using pauseViewportEvents before calling this function, then
 * resuming all viewport events with resumeViewportEvents after calling this function.  Since viewport events were
 * disabled during the changes, determine what viewports have changes by using the viewportHasChanges function and if so
 * refresh the ones that have changes.
 */
export async function replaceSession(
  session_id: string,
  pattern: string | Uint8Array,
  replacement: string | Uint8Array,
  is_case_insensitive: boolean | undefined,
  offset: number | undefined,
  length: number | undefined,
  limit: number | undefined
): Promise<number> {
  const foundLocations = await searchSession(
    session_id,
    pattern,
    is_case_insensitive,
    offset,
    length,
    limit
  )
  const patternArray =
    typeof pattern == 'string' ? Buffer.from(pattern) : pattern
  const replacementArray =
    typeof replacement == 'string' ? Buffer.from(replacement) : replacement
  // do replacements starting with the highest offset to the lowest offset, so offset adjustments don't need to be made
  for (let i = foundLocations.length - 1; i >= 0; --i) {
    await replaceOptimized(
      session_id,
      foundLocations[i],
      patternArray,
      replacementArray
    )
  }
  return foundLocations.length
}

/**
 * Replace found patterns in a segment in session iteratively
 * @param session_id session to replace patterns in
 * @param pattern pattern to replace
 * @param replacement replacement
 * @param is_case_insensitive false for case-sensitive matching and true for case-insensitive matching
 * @param offset start searching at this offset within the session, or at the start of the session if undefined
 * @param length search from the starting offset within the session up to this many bytes, if set to zero or undefined,
 * it will search to the end of the session
 * @return true of a replacement took place (false otherwise), and the offset to use for the next iteration (or -1 if no
 * replacement took place)
 */
export async function replaceOneSession(
  session_id: string,
  pattern: string | Uint8Array,
  replacement: string | Uint8Array,
  is_case_insensitive: boolean,
  offset: number | undefined,
  length: number | undefined
): Promise<[boolean, number]> {
  const patternArray =
    typeof pattern == 'string' ? Buffer.from(pattern) : pattern
  const replacementArray =
    typeof replacement == 'string' ? Buffer.from(replacement) : replacement
  const foundLocations = await searchSession(
    session_id,
    patternArray,
    is_case_insensitive,
    offset,
    length,
    1
  )
  if (foundLocations.length > 0) {
    await replaceOptimized(
      session_id,
      foundLocations[0],
      patternArray,
      replacementArray
    )
    // the next iteration offset should be at the end of this replacement
    return [true, foundLocations[0] + replacementArray.length]
  }
  return [false, -1]
}
