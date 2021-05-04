/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_PACING_BRAVE_TODAY_ADS_BRAVE_TODAY_AD_PACING_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_PACING_BRAVE_TODAY_ADS_BRAVE_TODAY_AD_PACING_H_

#include "bat/ads/internal/bundle/creative_brave_today_ad_info.h"

namespace ads {
namespace brave_today_ads {

CreativeBraveTodayAdList PaceAds(const CreativeBraveTodayAdList& ads);

}  // namespace brave_today_ads
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_PACING_BRAVE_TODAY_ADS_BRAVE_TODAY_AD_PACING_H_
