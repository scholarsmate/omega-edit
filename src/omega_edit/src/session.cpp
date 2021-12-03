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

#include "../include/session.h"
#include "impl_/change_def.h"
#include "impl_/model_def.h"
#include "impl_/session_def.h"
#include <cassert>

void *omega_session_get_user_data(const omega_session_t *session_ptr) { return session_ptr->user_data_ptr; }

size_t omega_session_get_num_viewports(const omega_session_t *session_ptr) { return session_ptr->viewports_.size(); }

int64_t omega_session_get_computed_file_size(const omega_session_t *session_ptr) {
    const auto computed_file_size = (session_ptr->model_ptr_->model_segments.empty())
                                            ? 0
                                            : session_ptr->model_ptr_->model_segments.back()->computed_offset +
                                                      session_ptr->model_ptr_->model_segments.back()->computed_length;
    assert(0 <= computed_file_size);
    return computed_file_size;
}

size_t omega_session_get_num_changes(const omega_session_t *session_ptr) {
    return session_ptr->model_ptr_->changes.size();
}

size_t omega_session_get_num_undone_changes(const omega_session_t *session_ptr) {
    return session_ptr->model_ptr_->changes_undone.size();
}

const omega_change_t *omega_session_get_last_change(const omega_session_t *session_ptr) {
    return (session_ptr->model_ptr_->changes.empty()) ? nullptr : session_ptr->model_ptr_->changes.back().get();
}

const omega_change_t *omega_session_get_last_undo(const omega_session_t *session_ptr) {
    return (session_ptr->model_ptr_->changes_undone.empty()) ? nullptr
                                                             : session_ptr->model_ptr_->changes_undone.back().get();
}

const char *omega_session_get_file_path(const omega_session_t *session_ptr) {
    return (session_ptr->file_path.empty()) ? nullptr : session_ptr->file_path.c_str();
}

int omega_session_visit_changes(const omega_session_t *session_ptr, omega_session_change_visitor_cbk_t cbk,
                                void *user_data) {
    int rc = 0;
    for (const auto &iter : session_ptr->model_ptr_->changes) {
        if ((rc = cbk(iter.get(), user_data)) != 0) { break; }
    }
    return rc;
}

int omega_session_visit_changes_reverse(const omega_session_t *session_ptr, omega_session_change_visitor_cbk_t cbk,
                                        void *user_data) {
    int rc = 0;
    for (auto iter = session_ptr->model_ptr_->changes.rbegin(); iter != session_ptr->model_ptr_->changes.rend();
         ++iter) {
        if ((rc = cbk(iter->get(), user_data)) != 0) { break; }
    }
    return rc;
}
