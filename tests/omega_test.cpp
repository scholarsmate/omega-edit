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

#define CATCH_CONFIG_MAIN

#include "../omega_edit/omega_edit.h"
#include "../omega_edit/omega_util.h"
#include "catch.hpp"
#include "test_util.h"

#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;

TEST_CASE("Buffer Shift", "[BufferShift]") {
    auto const fill = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    auto *buffer = (byte_t *) strdup(fill);
    auto buff_len = (int64_t) strlen((const char *) buffer);

    // Shift the buffer 3 bits to the right
    auto rc = right_shift_buffer(buffer, buff_len, 3);
    REQUIRE(rc == 0);
    // Shift the buffer 5 bits to the right
    rc = right_shift_buffer(buffer, buff_len, 5);
    REQUIRE(rc == 0);
    // We shifted a total of 8 bits (one byte) to the right, so compare the buffer against the fill plus one byte
    REQUIRE(strcmp((const char *) fill + 1, (const char *) buffer) == 0);

    // Reset the buffer
    memcpy(buffer, fill, buff_len);
    REQUIRE(strcmp((const char *) fill, (const char *) buffer) == 0);

    // Shift the buffer 6 bits to the left
    rc = left_shift_buffer(buffer, buff_len, 6);
    REQUIRE(rc == 0);
    // Shift the buffer 2 bits to the left
    rc = left_shift_buffer(buffer, buff_len, 2);
    REQUIRE(rc == 0);
    // We shifted a total of 8 bits (one byte) to the left, so compare the buffer against the fill plus one byte
    REQUIRE(strcmp((const char *) fill + 1, (const char *) buffer) == 0);

    free(buffer);
}

TEST_CASE("File Compare", "[UtilTests]") {
    SECTION("Identity") {
        // Same file ought to yield identical contents
        REQUIRE(compare_files("data/test1.dat", "data/test1.dat") == 0);
    }SECTION("Difference") {
        // Different files with different contents
        REQUIRE(compare_files("data/test1.dat", "data/test2.dat") == 1);
    }
}

TEST_CASE("Write Segment", "[WriteSegmentTests]") {
    FILE *test_outfile_ptr = fopen("data/test1.dat.seg", "w");
    FILE *read_file_ptr = fopen("data/test1.dat", "r");
    auto rc = write_segment_to_file(read_file_ptr, 10, 26, test_outfile_ptr);
    REQUIRE(rc == 26);
    rc = write_segment_to_file(read_file_ptr, 0, 10, test_outfile_ptr);
    REQUIRE(rc == 10);
    rc = write_segment_to_file(read_file_ptr, 36, 27, test_outfile_ptr);
    REQUIRE(rc == 27);
    fclose(read_file_ptr);
    fclose(test_outfile_ptr);
}

typedef struct file_info_struct {
    size_t num_changes{};
    char const *in_filename = nullptr;
} file_info_t;

void session_change_cbk(const session_t *session_ptr, const change_t *change_ptr) {
    auto file_info_ptr = (file_info_t *) get_session_user_data(session_ptr);
    const byte_t *bytes;
    const auto length = get_change_bytes(change_ptr, &bytes);
    if (0 < get_change_serial(change_ptr)) { ++file_info_ptr->num_changes; }
    else { --file_info_ptr->num_changes; /* this is in UNDO */}
    clog << dec << R"({ "filename" : ")" << file_info_ptr->in_filename << R"(", "num_changes" : )"
         << get_session_num_changes(session_ptr) << R"(, "computed_file_size": )" << get_computed_file_size(session_ptr)
         << R"(, "change_serial": )" << get_change_serial(change_ptr) << R"(, "kind": ")"
         << get_change_kind_as_char(change_ptr) << R"(", "offset": )" << get_change_offset(change_ptr)
         << R"(, "length": )" << get_change_length(change_ptr);
    if (bytes) {
        clog << R"(, "bytes": ")" << string((const char *) bytes, length) << R"(")";
    }
    clog << "}" << endl;
}

TEST_CASE("Empty File Test", "[EmptyFileTests]") {
    file_info_t file_info;
    file_info.num_changes = 0;
    file_info.in_filename = "data/empty_file.txt";
    const auto test_infile_fptr = fopen(file_info.in_filename, "r");
    REQUIRE(test_infile_fptr);
    const auto author_name = "empty file test";
    const auto session_ptr =
            create_session(test_infile_fptr, session_change_cbk, &file_info, DEFAULT_VIEWPORT_MAX_CAPACITY, 0, 0);
    const auto author_ptr = create_author(session_ptr, author_name);
    REQUIRE(get_session_offset(session_ptr) == 0);
    auto file_size = get_session_length(session_ptr);
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    ins(author_ptr, 0, reinterpret_cast<const byte_t *>("0"));
    file_size += 1;
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    destroy_session(session_ptr);
    fclose(test_infile_fptr);
}

TEST_CASE("Model Test", "[ModelTests]") {
    file_info_t file_info;
    file_info.num_changes = 0;
    file_info.in_filename = "data/model-test.txt";
    auto test_infile_fptr = fopen(file_info.in_filename, "r");
    REQUIRE(test_infile_fptr);
    const auto author_name = "model insert test";
    const auto session_ptr =
            create_session(test_infile_fptr, session_change_cbk, &file_info, DEFAULT_VIEWPORT_MAX_CAPACITY, 0, 0);
    fseeko(test_infile_fptr, 0, SEEK_END);
    auto file_size = ftello(test_infile_fptr);
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    auto author_ptr = create_author(session_ptr, author_name);
    ins(author_ptr, 0, reinterpret_cast<const byte_t *>("0"), 1);
    file_size += 1;
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    auto test_outfile_fptr = fopen("data/model-test.actual.1.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.txt", "data/model-test.actual.1.txt") != 0);
    REQUIRE(compare_files("data/model-test.expected.1.txt", "data/model-test.actual.1.txt") == 0);
    ins(author_ptr, 10, reinterpret_cast<const byte_t *>("0"), 1);
    file_size += 1;
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    test_outfile_fptr = fopen("data/model-test.actual.2.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.expected.2.txt", "data/model-test.actual.2.txt") == 0);
    ins(author_ptr, 5, reinterpret_cast<const byte_t *>("xxx"));
    file_size += 3;
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    test_outfile_fptr = fopen("data/model-test.actual.3.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.expected.3.txt", "data/model-test.actual.3.txt") == 0);
    auto num_changes = file_info.num_changes;
    undo_last_change(author_ptr);
    REQUIRE(file_info.num_changes == num_changes - 1);
    file_size -= 3;
    REQUIRE(get_computed_file_size(session_ptr) == file_size);
    test_outfile_fptr = fopen("data/model-test.actual.4.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.expected.4.txt", "data/model-test.actual.4.txt") == 0);
    ovr(author_ptr, 0, reinterpret_cast<const byte_t *>("-"));
    ovr(author_ptr, file_size - 1, reinterpret_cast<const byte_t *>("+"), 1);
    ins(author_ptr, 5, reinterpret_cast<const byte_t *>("XxXxXxX"), 7);
    del(author_ptr, 7, 4);
    ovr(author_ptr, 6, reinterpret_cast<const byte_t *>("O"), 0);
    test_outfile_fptr = fopen("data/model-test.actual.5.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.expected.5.txt", "data/model-test.actual.5.txt") == 0);
    del(author_ptr, 0, get_computed_file_size(session_ptr));
    REQUIRE(get_computed_file_size(session_ptr) == 0);
    while (file_info.num_changes) { undo_last_change(author_ptr); }
    test_outfile_fptr = fopen("data/model-test.actual.6.txt", "w");
    REQUIRE(test_outfile_fptr);
    save_to_file(session_ptr, test_outfile_fptr);
    REQUIRE(file_info.num_changes == get_session_num_changes(session_ptr));
    fclose(test_outfile_fptr);
    REQUIRE(compare_files("data/model-test.txt", "data/model-test.actual.6.txt") == 0);
    destroy_session(session_ptr);
    fclose(test_infile_fptr);
}

TEST_CASE("Check initialization", "[InitTests]") {
    FILE *test_infile_ptr;
    session_t *session_ptr;
    file_info_t file_info;
    const author_t *author_ptr;

    file_info.in_filename = "data/test1.dat";

    SECTION("Open data file") {
        test_infile_ptr = fopen(file_info.in_filename, "r");
        FILE *test_outfile_ptr = fopen("data/test1.dat.out", "w");
        REQUIRE(test_infile_ptr != NULL);
        SECTION("Create Session") {
            session_ptr = create_session(test_infile_ptr, session_change_cbk, &file_info, DEFAULT_VIEWPORT_MAX_CAPACITY,
                                         0, 0);
            REQUIRE(session_ptr != NULL);
            REQUIRE(get_computed_file_size(session_ptr) == 63);
            SECTION("Add Author") {
                const char *author_name = "Test Author";
                author_ptr = create_author(session_ptr, author_name);
                REQUIRE(strcmp(author_name, get_author_name(author_ptr)) == 0);
                SECTION("Add bytes") {
                    ins(author_ptr, 10, reinterpret_cast<const byte_t *>("++++"), 4);
                    REQUIRE(get_computed_file_size(session_ptr) == 67);
                    ovr(author_ptr, 12, reinterpret_cast<const byte_t *>("."), 1);
                    REQUIRE(get_computed_file_size(session_ptr) == 67);
                    ins(author_ptr, 0, reinterpret_cast<const byte_t *>("+++"));
                    REQUIRE(get_computed_file_size(session_ptr) == 70);
                    ovr(author_ptr, 1, reinterpret_cast<const byte_t *>("."));
                    REQUIRE(get_computed_file_size(session_ptr) == 70);
                    ovr(author_ptr, 15, reinterpret_cast<const byte_t *>("*"));
                    REQUIRE(get_computed_file_size(session_ptr) == 70);
                    ins(author_ptr, 15, reinterpret_cast<const byte_t *>("+"));
                    REQUIRE(get_computed_file_size(session_ptr) == 71);
                    del(author_ptr, 9, 5);
                    REQUIRE(get_computed_file_size(session_ptr) == 66);
                    auto num_changes_before_undo = get_session_num_changes(session_ptr);
                    REQUIRE(get_author_num_changes(author_ptr) == num_changes_before_undo);
                    undo_last_change(author_ptr);
                    REQUIRE(get_session_num_changes(session_ptr) == num_changes_before_undo - 1);
                    REQUIRE(get_computed_file_size(session_ptr) == 71);
                    save_to_file(session_ptr, test_outfile_ptr);
                    fclose(test_infile_ptr);
                    fclose(test_outfile_ptr);
                }
            }
            destroy_session(session_ptr);
        }
    }
}

enum display_mode_t {
    BIT_MODE, BYTE_MODE, CHAR_MODE
};
struct view_mode_t {
    enum display_mode_t display_mode = CHAR_MODE;
};

void vpt_change_cbk(const viewport_t *viewport_ptr, const change_t *change_ptr) {
    if (change_ptr) { clog << "Change Author: " << get_author_name(get_change_author(change_ptr)) << endl; }
    clog << "'" << get_author_name(get_viewport_author(viewport_ptr))
         << "' viewport, capacity: " << get_viewport_capacity(viewport_ptr)
         << " length: " << get_viewport_length(viewport_ptr)
         << " offset: " << get_viewport_computed_offset(viewport_ptr)
         << " bit offset:" << get_viewport_bit_offset(viewport_ptr) << endl;
    if (get_viewport_user_data(viewport_ptr)) {
        auto const *view_mode_ptr = (const view_mode_t *) get_viewport_user_data(viewport_ptr);
        switch (view_mode_ptr->display_mode) {
            case BIT_MODE:
                clog << " BIT MODE [";
                write_pretty_bits(get_viewport_data(viewport_ptr), get_viewport_length(viewport_ptr));
                clog << "]\n";
                break;
            case CHAR_MODE:
                clog << "CHAR MODE [";
                clog << string((const char *) get_viewport_data(viewport_ptr), get_viewport_length(viewport_ptr));
                clog << "]\n";
                break;
            default:// flow through
            case BYTE_MODE:
                clog << "BYTE MODE [";
                write_pretty_bytes(get_viewport_data(viewport_ptr), get_viewport_length(viewport_ptr));
                clog << "]\n";
                break;
        }
        clog << endl;
    }
}

TEST_CASE("File Viewing", "[InitTests]") {
    auto const fill = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    auto const fill_length = static_cast<int64_t>(strlen(fill));
    auto const file_name = "data/test.dat.view";
    auto const *author_name = "Test Author";
    auto test_infile_ptr = fill_file(file_name, 1024, fill, fill_length);
    session_t *session_ptr;
    const author_t *author_ptr;
    viewport_t *viewport_ptr;
    view_mode_t view_mode;

    session_ptr = create_session(test_infile_ptr, nullptr, nullptr, 100, 0, 0);
    REQUIRE(get_session_viewport_max_capacity(session_ptr) == 100);
    author_ptr = create_author(session_ptr, author_name);
    auto viewport_count = get_session_num_viewports(session_ptr);
    REQUIRE(viewport_count == 0);
    view_mode.display_mode = BIT_MODE;
    viewport_ptr = create_viewport(author_ptr, 0, 10, vpt_change_cbk, &view_mode, 0);
    REQUIRE(viewport_count + 1 == get_session_num_viewports(session_ptr));
    view_mode.display_mode = CHAR_MODE;
    vpt_change_cbk(viewport_ptr, nullptr);
    for (int64_t offset(0); offset < get_computed_file_size(session_ptr); ++offset) {
        update_viewport(viewport_ptr, offset, 10 + (offset % 40), 0);
    }

    // Change the display mode from character mode to byte mode to handle non-standard byte alignment

    view_mode.display_mode = BIT_MODE;
    update_viewport(viewport_ptr, 0, 20, 0);

    // Change to bit offsets
    update_viewport(viewport_ptr, 0, 20, 1);
    update_viewport(viewport_ptr, 0, 20, 2);
    update_viewport(viewport_ptr, 0, 20, 3);
    update_viewport(viewport_ptr, 0, 20, 4);
    update_viewport(viewport_ptr, 0, 20, 5);
    update_viewport(viewport_ptr, 0, 20, 6);
    update_viewport(viewport_ptr, 0, 20, 7);

    view_mode.display_mode = BYTE_MODE;
    vpt_change_cbk(viewport_ptr, nullptr);

    // Copy the contents of the 6-bit offset viewport into a buffer and shift it 1 more bit to get back on 8-bit
    // alignment for simple comparison with the original fill
    auto *buffer = (byte_t *) malloc(get_viewport_capacity(viewport_ptr));
    memcpy(buffer, get_viewport_data(viewport_ptr), get_viewport_length(viewport_ptr));
    auto rc = left_shift_buffer(buffer, get_viewport_length(viewport_ptr), 1);
    REQUIRE(rc == 0);
    REQUIRE(memcmp(buffer, fill + 1, get_viewport_length(viewport_ptr) - 1) == 0);
    free(buffer);

    rc = ins(author_ptr, 3, reinterpret_cast<const byte_t *>("++++"));
    REQUIRE(rc == 0);
    viewport_count = get_session_num_viewports(session_ptr);
    rc = destroy_viewport(viewport_ptr);
    REQUIRE(rc == 0);
    REQUIRE(viewport_count - 1 == get_session_num_viewports(session_ptr));
    destroy_session(session_ptr);
    fclose(test_infile_ptr);
    remove(file_name);
}
