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
#ifndef OMEGA_EDIT_TEST_UTIL_H
#define OMEGA_EDIT_TEST_UTIL_H

#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

// define DEBUG for debugging
#define DEBUG

#ifdef DEBUG
#define DBG(x)                                                                                                         \
    do { x } while (0)
#else
#define DBG(x)
#endif

// Returns 0 if the content of the 2 file pointers are the same (from where the pointers are currently) and 1 if contents are not the same
inline int compare_file_pointers(FILE *f1, FILE *f2) {
    const size_t buff_size = 1024 * 8;
    byte_t buf1[buff_size];
    byte_t buf2[buff_size];

    do {
        auto r1 = fread(buf1, 1, buff_size, f1);
        auto r2 = fread(buf2, 1, buff_size, f2);

        if (r1 != r2 || memcmp(buf1, buf2, r1) != 0) {
            return 1;// Files are not equal
        }
    } while (!feof(f1) && !feof(f2));

    return (feof(f1) && feof(f2)) ? 0 : 1;
}

inline int compare_files(const char *f1, const char *f2) {
    const auto f1_ptr = fopen(f1, "r");
    const auto f2_ptr = fopen(f2, "r");
    const auto result = compare_file_pointers(f1_ptr, f2_ptr);
    fclose(f1_ptr);
    fclose(f2_ptr);
    return result;
}

inline FILE *fill_file(const char *f1, int64_t file_size, const char *fill, int64_t fill_length) {
    const auto f1_ptr = fopen(f1, "w+");
    while (file_size) {
        const auto count = (fill_length > file_size) ? file_size : fill_length;
        if (count != fwrite(fill, 1, count, f1_ptr)) { abort(); }
        file_size -= count;
    }
    fflush(f1_ptr);
    fseek(f1_ptr, 0, SEEK_SET);
    return f1_ptr;
}

inline void write_pretty_bits_byte(byte_t byte) {
    for (auto i = 7; 0 <= i; --i) { clog << ((byte & (1 << i)) ? '1' : '0'); }
}

inline void write_pretty_bits(const byte_t *ptr, int64_t size) {
    if (size > 0) {
        auto i = 0;
        write_pretty_bits_byte(ptr[i++]);
        while (i < size) {
            clog << " ";
            write_pretty_bits_byte(ptr[i++]);
        }
    }
}

inline void write_pretty_bytes(const byte_t *data, int64_t size) {
    if (size > 0) {
        auto i = 0;
        clog << setfill('0');
        clog << hex << setw(2) << static_cast<int>(data[i++]);
        while (i < size) { clog << " " << hex << setw(2) << (int) data[i++]; }
    }
}

#endif//OMEGA_EDIT_TEST_UTIL_H
