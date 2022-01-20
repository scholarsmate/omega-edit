/**********************************************************************************************************************
 * Copyright (c) 2021-2022 Concurrent Technologies Corporation.                                                       *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance     *
 * with the License.  You may obtain a copy of the License at                                                         *
 *                                                                                                                    *
 *     http://www.apache.org/licenses/LICENSE-2.0                                                                     *
 *                                                                                                                    *
 * Unless required by applicable law or agreed to in writing, software is distributed under the License is            *
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or                   *
 * implied.  See the License for the specific language governing permissions and limitations under the License.       *
 *                                                                                                                    *
 **********************************************************************************************************************/

#include "../include/omega_edit/config.h"

#ifdef OMEGA_BUILD_WINDOWS
#include <direct.h>
#include <io.h>
#include <objbase.h>
#include <process.h>
#include <sys/utime.h>
#ifdef OPEN
#undef OPEN
#endif
#define OPEN _open
#define O_CREAT _O_CREAT
#define O_RDWR _O_RDWR
#define close _close
#define getcwd _getcwd
#define getpid _getpid
#define utime _utime
#else
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utime.h>
#include <uuid/uuid.h>
#ifndef O_BINARY
#define O_BINARY (0)
#endif
#endif

#include "../include/omega_edit/utility.h"
#include "impl_/macros.h"
#include <assert.h>
#include <cwalk.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef OMEGA_BUILD_WINDOWS
char *omega_util_create_uuid(char *buffer) {
    static char buff[UUID_STRING_LEN];
    GUID out = {0};
    CoCreateGuid(&out);
    if (!buffer) { buffer = buff; }
    sprintf(buffer, "%08X-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X", out.Data1, out.Data2, out.Data3, out.Data4[0],
            out.Data4[1], out.Data4[2], out.Data4[3], out.Data4[4], out.Data4[5], out.Data4[6], out.Data4[7]);
    for (; *buffer; ++buffer) *buffer = tolower(*buffer);
    return buffer;
}
#else
char *omega_util_create_uuid(char *buffer) {
    static char buff[UUID_STRING_LEN];
    uuid_t out;
    if (!buffer) { buffer = buff; }
    uuid_generate(out);
    uuid_unparse_lower(out, buffer);
    return buffer;
}
#endif

int omega_util_mkstemp(char *tmpl) {
    static const char letters[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";//len = 62
    static uint64_t value;
    const size_t len = strlen(tmpl);
    char *template;
    int count, fd;
    int saved_errno = errno;

    if (len < 6 || 0 != strcmp(&tmpl[len - 6], "XXXXXX")) {
        errno = EINVAL;
        return -1;
    }

    // This is where the Xs start.
    template = &tmpl[len - 6];

#ifdef OMEGA_BUILD_WINDOWS
    value += rand();
    value += ((value << 32) + rand()) ^ getpid();
#else
    value += random() ^ getpid();
#endif

    for (count = 0; count < TMP_MAX; value += 7777, ++count) {
        uint64_t v = value;

        // Fill in the random bits.
        template[0] = letters[v % 62];
        v /= 62;
        template[1] = letters[v % 62];
        v /= 62;
        template[2] = letters[v % 62];
        v /= 62;
        template[3] = letters[v % 62];
        v /= 62;
        template[4] = letters[v % 62];
        v /= 62;
        template[5] = letters[v % 62];

        fd = OPEN(tmpl, O_RDWR | O_CREAT | O_EXCL | O_BINARY, 0600);
        if (fd >= 0) {
            errno = saved_errno;
            return fd;
        } else if (errno != EEXIST)
            // Any other error will apply to other names we might try, and there are about 2^32 of them, so give up.
            return -1;
    }

    // We got out of the loop because we ran out of combinations to try.
    errno = EEXIST;
    return -1;
}

const char *omega_util_get_current_dir(char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold path
    if (!buffer) { buffer = buff; }
    buffer[0] = '\0';
    return (getcwd(buffer, FILENAME_MAX)) ? buffer : NULL;
}

int omega_util_touch(const char *file_name, int create) {
    int fd = OPEN(file_name, (create) ? O_RDWR | O_CREAT : O_RDWR, 0644);
    if (fd < 0) {
        if (!create && errno == ENOENT) {
            return 0;
        } else {
            DBG(perror("omega_util_touch"););
            return -1;
        }
    }
    close(fd);
    if (utime(file_name, NULL)) {
        DBG(perror("omega_util_touch"););
        return -1;
    }
    return 0;
}

int omega_util_file_exists(const char *file_name) {
    assert(file_name);
    FILE *file_ptr = fopen(file_name, "rb");
    if (file_ptr) {
        fclose(file_ptr);
        return 1;
    }
    return 0;
}

char omega_util_directory_separator() {
#ifdef OMEGA_BUILD_WINDOWS
    return '\\';
#else
    return '/';
#endif
}

char *omega_util_dirname(char const *path, char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold directory name
    assert(path);
    if (!buffer) { buffer = buff; }
    size_t dirname_len;
    cwk_path_get_dirname(path, &dirname_len);
    memcpy(buffer, path, dirname_len);
    buffer[dirname_len] = '\0';
    return buffer;
}

char *omega_util_basename(char const *path, char const *suffix, char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold basename
    assert(path);
    if (!buffer) { buffer = buff; }
    const char *basename;
    size_t basename_len;
    cwk_path_get_basename(path, &basename, &basename_len);
    if (!basename) { return NULL; }
    memcpy(buffer, basename, basename_len);
    buffer[basename_len] = '\0';
    if (suffix) {
        const size_t suffix_len = strlen(suffix);
        if (suffix_len < basename_len && 0 == strncmp(buffer + basename_len - suffix_len, suffix, suffix_len)) {
            buffer[basename_len - suffix_len] = '\0';
        }
    }
    return buffer;
}

char *omega_util_file_extension(char const *path, char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold extension
    char file_name_buff[FILENAME_MAX];
    path = omega_util_basename(path, NULL, file_name_buff);
    assert(path);
    if (!buffer) { buffer = buff; }
    const char *extension;
    size_t extension_len;
    if (cwk_path_get_extension(path, &extension, &extension_len)) {
        memcpy(buffer, extension, extension_len);
        buffer[extension_len] = '\0';
        return buffer;
    }
    buffer[0] = '\0';
    return NULL;
}

char *omega_util_normalize_path(char const *path, char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold path
    assert(path);
    if (!buffer) { buffer = buff; }
    cwk_path_normalize(path, buffer, FILENAME_MAX);
    return buffer;
}

char *omega_util_available_filename(char const *path, char *buffer) {
    static char buff[FILENAME_MAX];//create string buffer to hold path
    assert(path);
    if (!buffer) { buffer = buff; }
    if (!omega_util_file_exists(path)) {
        memcpy(buffer, path, strlen(path) + 1);
        return buffer;
    }
    int i = 0;
    const char *dirname = omega_util_dirname(path, NULL);
    const char *extension = omega_util_file_extension(path, NULL);
    const char *basename = omega_util_basename(path, extension, NULL);
    do {
        if (i == 99) {
            // stop after 99 filenames exist
            return NULL;
        }
        snprintf(buffer, FILENAME_MAX, "%s%s-%d%s", dirname, basename, ++i, extension);
    } while (omega_util_file_exists(buffer));
    return buffer;
}

int64_t omega_util_write_segment_to_file(FILE *from_file_ptr, int64_t offset, int64_t byte_count, FILE *to_file_ptr) {
    assert(from_file_ptr);
    assert(to_file_ptr);
    if (0 != FSEEK(from_file_ptr, offset, SEEK_SET)) { return -1; }
    int64_t remaining = byte_count;
    omega_byte_t buff[1024 * 8];
    while (remaining) {
        const int64_t count = ((int64_t) sizeof(buff) > remaining) ? remaining : (int64_t) sizeof(buff);
        if (count != (int64_t) fread(buff, sizeof(omega_byte_t), count, from_file_ptr) ||
            count != (int64_t) fwrite(buff, sizeof(omega_byte_t), count, to_file_ptr)) {
            break;
        }
        remaining -= count;
    }
    return byte_count - remaining;
}

int omega_util_left_shift_buffer(omega_byte_t *buffer, int64_t len, omega_byte_t shift_left) {
    assert(buffer);
    if (shift_left > 0 && shift_left < 8) {
        omega_byte_t shift_right = 8 - shift_left;
        omega_byte_t mask = ((1 << shift_left) - 1) << shift_right;
        omega_byte_t bits1 = 0;
        int64_t i;
        for (i = len - 1; i >= 0; --i) {
            const unsigned char bits2 = buffer[i] & mask;
            buffer[i] <<= shift_left;
            buffer[i] |= bits1 >> shift_right;
            bits1 = bits2;
        }
        return 0;
    }
    return -1;
}

int omega_util_right_shift_buffer(omega_byte_t *buffer, int64_t len, omega_byte_t shift_right) {
    assert(buffer);
    if (shift_right > 0 && shift_right < 8) {
        omega_byte_t shift_left = 8 - shift_right;
        omega_byte_t mask = (1 << shift_right) - 1;
        omega_byte_t bits1 = 0;
        int64_t i;
        for (i = len - 1; i >= 0; --i) {
            const unsigned char bits2 = buffer[i] & mask;
            buffer[i] >>= shift_right;
            buffer[i] |= bits1 << shift_left;
            bits1 = bits2;
        }
        return 0;
    }
    return -1;
}

void omega_util_apply_byte_transform(omega_byte_t *buffer, int64_t len, omega_util_byte_transform_t transform,
                                     void *user_data_ptr) {
    assert(buffer);
    int64_t i;
    for (i = 0; i < len; ++i) { buffer[i] = transform(buffer[i], user_data_ptr); }
}

int omega_util_apply_byte_transform_to_file(char const *in_path, char const *out_path,
                                            omega_util_byte_transform_t transform, void *user_data_ptr, int64_t offset,
                                            int64_t length) {
    assert(in_path);
    assert(out_path);
    assert(0 <= offset);
    assert(0 <= length);
    FILE *in_fp = fopen(in_path, "rb");
    assert(in_fp);
    FSEEK(in_fp, 0, SEEK_END);
    int64_t in_file_length = FTELL(in_fp);
    if (0 == length) { length = in_file_length - offset; }
    do {
        if (length < 1 || in_file_length <= offset || in_file_length < offset + length) { break; }
        FILE *out_fp = fopen(out_path, "wb");
        assert(out_fp);
        if (omega_util_write_segment_to_file(in_fp, 0, offset, out_fp) != offset ||
            0 != FSEEK(in_fp, offset, SEEK_SET)) {
            LOG_ERROR("failed to write first segment bytes to file");
            fclose(out_fp);
            unlink(out_path);
            break;
        }
        int64_t remaining = length;
        omega_byte_t buff[1024 * 8];
        while (remaining) {
            const int64_t count = ((int64_t) sizeof(buff) > remaining) ? remaining : (int64_t) sizeof(buff);
            const int64_t num_read = (int64_t) fread(buff, sizeof(omega_byte_t), count, in_fp);
            if (count != num_read) {
                LOG_ERROR("failed to read buffer");
                break;
            }
            omega_util_apply_byte_transform(buff, count, transform, user_data_ptr);
            const int64_t num_written = (int64_t) fwrite(buff, sizeof(omega_byte_t), count, out_fp);
            if (count != num_written) {
                LOG_ERROR("failed to write buffer");
                break;
            }
            remaining -= count;
        }
        if (remaining) {
            LOG_ERROR("there are remaining bytes");
            fclose(out_fp);
            unlink(out_path);
            break;
        }
        offset += length;
        length = in_file_length - offset;
        if (offset < in_file_length && omega_util_write_segment_to_file(in_fp, offset, length, out_fp) != length) {
            LOG_ERROR("failed to write last segment");
            fclose(out_fp);
            unlink(out_path);
            break;
        }
        fclose(out_fp);
        fclose(in_fp);
        return 0;
    } while (false);
    fclose(in_fp);
    LOG_ERROR("transform failed");
    return -1;
}

omega_byte_t omega_util_mask_byte(omega_byte_t byte, omega_byte_t mask, omega_mask_kind_t mask_kind) {
    switch (mask_kind) {
        case MASK_AND:
            return byte & mask;
        case MASK_OR:
            return byte | mask;
        case MASK_XOR:
            return byte ^ mask;
        default:
            ABORT(LOG_ERROR("unhandled mask kind"););
    }
}
