/**********************************************************************************************************************
 * Copyright (c) 2021 Concurrent Technologies Corporation.                                                            *
 *                                                                                                                    *
 * Licensed under the Apache License, Version 2.0 (the "License");                                                    *
 * you may not use this file except in compliance with the License.                                                   *
 * You may obtain a copy of the License at                                                                            *
 *                                                                                                                    *
 *     http://www.apache.org/licenses/LICENSE-2.0                                                                     *
 *                                                                                                                    *
 * Unless required by applicable law or agreed to in writing, software                                                *
 * distributed under the License is distributed on an "AS IS" BASIS,                                                  *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.                                           *
 * See the License for the specific language governing permissions and                                                *
 * limitations under the License.                                                                                     *
 **********************************************************************************************************************/

#include "../omega_edit/omega_edit.h"
#include <string>

using namespace std;

int main(int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr,
                "This program extracts a slice from the infile and writes it to the outfile using an Omega Edit\n"
                "session.\n\n"
                "USAGE: %s infile outfile offset length\n",
                argv[0]);
        return -1;
    }
    const auto in_filename = argv[1];
    const auto out_filename = argv[2];
    const auto offset = stoll(argv[3]);
    const auto length = stoll(argv[4]);
    auto session_ptr = omega_edit_create_session(in_filename);
    if (session_ptr) {
        if (offset) { omega_edit_delete(session_ptr, 0, offset); }
        omega_edit_delete(session_ptr, length, omega_session_get_computed_file_size(session_ptr));
        omega_edit_save(session_ptr, out_filename);
        omega_edit_destroy_session(session_ptr);
    } else {
        fprintf(stderr, "failed to create session, probably because the offset and/or length are out of range for the\n"
                        "given input file\n");
    }
    return 0;
}