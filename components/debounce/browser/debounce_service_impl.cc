/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/debounce/browser/debounce_service_impl.h"

#include "brave/components/debounce/browser/debounce_download_service.h"

namespace debounce {

DebounceServiceImpl::DebounceServiceImpl(
    DebounceDownloadService* download_service,
    scoped_refptr<base::SequencedTaskRunner> task_runner)
    : download_service_(download_service), weak_factory_(this) {}

DebounceServiceImpl::~DebounceServiceImpl() {}

bool DebounceServiceImpl::Debounce(GURL url, GURL* final_url) {
  bool changed = false;
  std::vector<std::unique_ptr<DebounceRule>>* rules =
      download_service_->rules();
  for (const std::unique_ptr<DebounceRule>& rule : *rules) {
    if (rule->Apply(url, final_url)) {
      if (url != *final_url) {
        changed = true;
        url = *final_url;
      }
    }
  }
  return changed;
}

}  // namespace debounce
