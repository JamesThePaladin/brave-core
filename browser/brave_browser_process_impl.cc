/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_browser_process_impl.h"

#include <utility>

#include "base/bind.h"
#include "base/path_service.h"
#include "base/task/post_task.h"
#include "base/threading/sequenced_task_runner_handle.h"
#include "brave/browser/brave_stats_updater.h"
#include "brave/browser/component_updater/brave_component_updater_configurator.h"
#include "brave/browser/component_updater/brave_component_updater_delegate.h"
#include "brave/browser/net/brave_system_request_handler.h"
#include "brave/browser/profiles/brave_profile_manager.h"
#include "brave/browser/themes/brave_dark_mode_utils.h"
#include "brave/browser/tor/buildflags.h"
#include "brave/browser/ui/brave_browser_command_controller.h"
#include "brave/common/pref_names.h"
#include "brave/components/brave_component_updater/browser/brave_on_demand_updater.h"
#include "brave/components/brave_component_updater/browser/local_data_files_service.h"
#include "brave/components/brave_shields/browser/ad_block_custom_filters_service.h"
#include "brave/components/brave_shields/browser/ad_block_regional_service_manager.h"
#include "brave/components/brave_shields/browser/ad_block_service.h"
#include "brave/components/brave_shields/browser/https_everywhere_service.h"
#include "brave/components/brave_shields/browser/referrer_whitelist_service.h"
#include "brave/components/brave_shields/browser/tracking_protection_service.h"
#include "brave/components/ntp_sponsored_images/browser/features.h"
#include "brave/components/ntp_sponsored_images/browser/ntp_sponsored_images_service.h"
#include "brave/components/p3a/buildflags.h"
#include "brave/components/p3a/brave_histogram_rewrite.h"
#include "brave/components/p3a/brave_p3a_service.h"
#include "brave/services/network/public/cpp/system_request_handler.h"
#include "chrome/browser/ui/browser_list.h"
#include "chrome/browser/ui/webui/components/components_ui.h"
#include "chrome/common/buildflags.h"
#include "chrome/common/chrome_paths.h"
#include "components/component_updater/component_updater_service.h"
#include "components/component_updater/timer_update_scheduler.h"
#include "content/public/browser/browser_thread.h"
#include "services/network/public/cpp/resource_request.h"

#if BUILDFLAG(ENABLE_NATIVE_NOTIFICATIONS)
#include "chrome/browser/notifications/notification_platform_bridge.h"
#include "brave/browser/notifications/brave_notification_platform_bridge.h"
#endif

#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
#include "brave/browser/widevine/brave_widevine_bundle_manager.h"
#endif

#if BUILDFLAG(ENABLE_BRAVE_REFERRALS)
#include "brave/components/brave_referrals/browser/brave_referrals_service.h"
#endif

#if BUILDFLAG(ENABLE_EXTENSIONS)
#include "brave/common/extensions/whitelist.h"
#include "brave/components/brave_component_updater/browser/extension_whitelist_service.h"
#endif

#if BUILDFLAG(ENABLE_GREASELION)
#include "brave/components/greaselion/browser/greaselion_download_service.h"
#endif

#if BUILDFLAG(ENABLE_TOR)
#include "brave/browser/extensions/brave_tor_client_updater.h"
#include "brave/common/tor/pref_names.h"
#endif

#if BUILDFLAG(ENABLE_SPEEDREADER)
#include "brave/components/speedreader/speedreader_whitelist.h"
#endif

#if defined(OS_ANDROID)
#include "chrome/browser/android/chrome_feature_list.h"
#include "chrome/browser/android/component_updater/background_task_update_scheduler.h"
#else
#include "chrome/browser/ui/browser.h"
#endif

using brave_component_updater::BraveComponent;
using ntp_sponsored_images::features::kBraveNTPBrandedWallpaper;
using ntp_sponsored_images::NTPSponsoredImagesService;

namespace {

// Initializes callback for SystemRequestHandler
void InitSystemRequestHandlerCallback() {
  network::SystemRequestHandler::OnBeforeSystemRequestCallback
      before_system_request_callback = base::Bind(brave::OnBeforeSystemRequest);
  network::SystemRequestHandler::GetInstance()
      ->RegisterOnBeforeSystemRequestCallback(before_system_request_callback);
}

}  // namespace

BraveBrowserProcessImpl* g_brave_browser_process = nullptr;

using content::BrowserThread;

BraveBrowserProcessImpl::~BraveBrowserProcessImpl() {}

BraveBrowserProcessImpl::BraveBrowserProcessImpl(StartupData* startup_data)
    : BrowserProcessImpl(startup_data) {
  g_browser_process = this;
  g_brave_browser_process = this;

#if BUILDFLAG(ENABLE_BRAVE_REFERRALS)
  brave_referrals_service_ = brave::BraveReferralsServiceFactory(local_state());
  base::SequencedTaskRunnerHandle::Get()->PostDelayedTask(
      FROM_HERE,
      base::BindOnce(
          [](brave::BraveReferralsService* referrals_service) {
            referrals_service->Start();
          },
          base::Unretained(brave_referrals_service_.get())),
      base::TimeDelta::FromSeconds(3));
#endif

  base::SequencedTaskRunnerHandle::Get()->PostTask(
      FROM_HERE, base::BindOnce(
                     [](brave::BraveStatsUpdater* stats_updater) {
                       stats_updater->Start();
                     },
                     base::Unretained(brave_stats_updater())));
  // Disabled on mobile platforms, see for instance issues/6176
#if BUILDFLAG(BRAVE_P3A_ENABLED)
  // Create P3A Service early to catch more histograms. The full initialization
  // should be started once browser process impl is ready.
  brave_p3a_service();
  brave::SetupHistogramsBraveization();
#endif  // BUILDFLAG(BRAVE_P3A_ENABLED)
}

void BraveBrowserProcessImpl::Init() {
  BrowserProcessImpl::Init();

  brave_component_updater::BraveOnDemandUpdater::GetInstance()->
      RegisterOnDemandUpdateCallback(
          base::BindRepeating(&ComponentsUI::OnDemandUpdate));
  UpdateBraveDarkMode();
  pref_change_registrar_.Add(
      kBraveDarkMode,
      base::Bind(&BraveBrowserProcessImpl::OnBraveDarkModeChanged,
                 base::Unretained(this)));

#if BUILDFLAG(ENABLE_TOR)
  pref_change_registrar_.Add(
      tor::prefs::kTorDisabled,
      base::Bind(&BraveBrowserProcessImpl::OnTorEnabledChanged,
                 base::Unretained(this)));
#endif

  InitSystemRequestHandlerCallback();
}

brave_component_updater::BraveComponent::Delegate*
BraveBrowserProcessImpl::brave_component_updater_delegate() {
  if (!brave_component_updater_delegate_)
    brave_component_updater_delegate_ =
        std::make_unique<brave::BraveComponentUpdaterDelegate>();

  return brave_component_updater_delegate_.get();
}

ProfileManager* BraveBrowserProcessImpl::profile_manager() {
  DCHECK_CALLED_ON_VALID_SEQUENCE(sequence_checker_);
  if (!created_profile_manager_)
    CreateProfileManager();
  return profile_manager_.get();
}

void BraveBrowserProcessImpl::StartBraveServices() {
  DCHECK_CURRENTLY_ON(content::BrowserThread::UI);

  ad_block_service()->Start();
  ad_block_custom_filters_service()->Start();
  ad_block_regional_service_manager()->Start();
  https_everywhere_service()->Start();

#if BUILDFLAG(ENABLE_EXTENSIONS)
  extension_whitelist_service();
#endif
  referrer_whitelist_service();
  tracking_protection_service();
#if BUILDFLAG(ENABLE_GREASELION)
  greaselion_download_service();
#endif
#if BUILDFLAG(ENABLE_SPEEDREADER)
  speedreader_whitelist();
#endif
  // Now start the local data files service, which calls all observers.
  local_data_files_service()->Start();
}

brave_shields::AdBlockService* BraveBrowserProcessImpl::ad_block_service() {
  if (ad_block_service_)
    return ad_block_service_.get();

  ad_block_service_ =
      brave_shields::AdBlockServiceFactory(brave_component_updater_delegate());
  return ad_block_service_.get();
}

brave_shields::AdBlockCustomFiltersService*
BraveBrowserProcessImpl::ad_block_custom_filters_service() {
  if (!ad_block_custom_filters_service_)
    ad_block_custom_filters_service_ =
        brave_shields::AdBlockCustomFiltersServiceFactory(
            brave_component_updater_delegate());
  return ad_block_custom_filters_service_.get();
}

brave_shields::AdBlockRegionalServiceManager*
BraveBrowserProcessImpl::ad_block_regional_service_manager() {
  if (!ad_block_regional_service_manager_)
    ad_block_regional_service_manager_ =
        brave_shields::AdBlockRegionalServiceManagerFactory(
            brave_component_updater_delegate());
  return ad_block_regional_service_manager_.get();
}

NTPSponsoredImagesService*
BraveBrowserProcessImpl::ntp_sponsored_images_service() {
  if (!base::FeatureList::IsEnabled(kBraveNTPBrandedWallpaper))
    return nullptr;

  if (!ntp_sponsored_images_service_) {
    ntp_sponsored_images_service_ =
        std::make_unique<NTPSponsoredImagesService>(component_updater());
  }

  return ntp_sponsored_images_service_.get();
}

#if BUILDFLAG(ENABLE_EXTENSIONS)
brave_component_updater::ExtensionWhitelistService*
BraveBrowserProcessImpl::extension_whitelist_service() {
  if (!extension_whitelist_service_) {
    extension_whitelist_service_ =
        brave_component_updater::ExtensionWhitelistServiceFactory(
            local_data_files_service(), kVettedExtensions);
  }
  return extension_whitelist_service_.get();
}
#endif

brave_shields::ReferrerWhitelistService*
BraveBrowserProcessImpl::referrer_whitelist_service() {
  if (!referrer_whitelist_service_) {
    referrer_whitelist_service_ =
        brave_shields::ReferrerWhitelistServiceFactory(
            local_data_files_service());
  }
  return referrer_whitelist_service_.get();
}

#if BUILDFLAG(ENABLE_GREASELION)
greaselion::GreaselionDownloadService*
BraveBrowserProcessImpl::greaselion_download_service() {
  if (!greaselion_download_service_) {
    greaselion_download_service_ = greaselion::GreaselionDownloadServiceFactory(
        local_data_files_service());
  }
  return greaselion_download_service_.get();
}
#endif

brave_shields::TrackingProtectionService*
BraveBrowserProcessImpl::tracking_protection_service() {
  if (!tracking_protection_service_) {
    tracking_protection_service_ =
        brave_shields::TrackingProtectionServiceFactory(
            local_data_files_service());
  }
  return tracking_protection_service_.get();
}

brave_shields::HTTPSEverywhereService*
BraveBrowserProcessImpl::https_everywhere_service() {
  if (!https_everywhere_service_)
    https_everywhere_service_ = brave_shields::HTTPSEverywhereServiceFactory(
        brave_component_updater_delegate());
  return https_everywhere_service_.get();
}

brave_component_updater::LocalDataFilesService*
BraveBrowserProcessImpl::local_data_files_service() {
  if (!local_data_files_service_)
    local_data_files_service_ =
        brave_component_updater::LocalDataFilesServiceFactory(
            brave_component_updater_delegate());
  return local_data_files_service_.get();
}

void BraveBrowserProcessImpl::UpdateBraveDarkMode() {
  // Update with proper system theme to make brave theme and base ui components
  // theme use same theme.
  dark_mode::SetSystemDarkMode(dark_mode::GetBraveDarkModeType());
}

void BraveBrowserProcessImpl::OnBraveDarkModeChanged() {
  UpdateBraveDarkMode();
}

#if BUILDFLAG(ENABLE_TOR)
extensions::BraveTorClientUpdater*
BraveBrowserProcessImpl::tor_client_updater() {
  if (tor_client_updater_)
    return tor_client_updater_.get();

  tor_client_updater_ = extensions::BraveTorClientUpdaterFactory(
      brave_component_updater_delegate());
  return tor_client_updater_.get();
}

void BraveBrowserProcessImpl::OnTorEnabledChanged() {
  // Update all browsers' tor command status.
  for (Browser* browser : *BrowserList::GetInstance()) {
    static_cast<chrome::BraveBrowserCommandController*>(
        browser->command_controller())->UpdateCommandForTor();
  }
}
#endif

brave::BraveP3AService* BraveBrowserProcessImpl::brave_p3a_service() {
  if (brave_p3a_service_) {
    return brave_p3a_service_.get();
  }
  brave_p3a_service_ = new brave::BraveP3AService(local_state());
  brave_p3a_service()->InitCallbacks();
  return brave_p3a_service_.get();
}

#if BUILDFLAG(BUNDLE_WIDEVINE_CDM)
BraveWidevineBundleManager*
BraveBrowserProcessImpl::brave_widevine_bundle_manager() {
  if (!brave_widevine_bundle_manager_)
    brave_widevine_bundle_manager_.reset(new BraveWidevineBundleManager);
  return brave_widevine_bundle_manager_.get();
}
#endif

brave::BraveStatsUpdater* BraveBrowserProcessImpl::brave_stats_updater() {
  if (!brave_stats_updater_)
    brave_stats_updater_ = brave::BraveStatsUpdaterFactory(local_state());
  return brave_stats_updater_.get();
}

void BraveBrowserProcessImpl::CreateProfileManager() {
  DCHECK(!created_profile_manager_ && !profile_manager_);
  created_profile_manager_ = true;

  base::FilePath user_data_dir;
  base::PathService::Get(chrome::DIR_USER_DATA, &user_data_dir);
  profile_manager_ = std::make_unique<BraveProfileManager>(user_data_dir);
}

NotificationPlatformBridge*
BraveBrowserProcessImpl::notification_platform_bridge() {
#if !defined(OS_MACOSX)
  return BrowserProcessImpl::notification_platform_bridge();
#else
#if BUILDFLAG(ENABLE_NATIVE_NOTIFICATIONS)
  if (!created_notification_bridge_)
    CreateNotificationPlatformBridge();
  return notification_bridge_.get();
#else
  return nullptr;
#endif
#endif
}

void BraveBrowserProcessImpl::CreateNotificationPlatformBridge() {
#if defined(OS_MACOSX)
#if BUILDFLAG(ENABLE_NATIVE_NOTIFICATIONS)
  DCHECK(!notification_bridge_);
  notification_bridge_ = BraveNotificationPlatformBridge::Create();
  created_notification_bridge_ = true;
#endif
#endif
}

#if BUILDFLAG(ENABLE_SPEEDREADER)
speedreader::SpeedreaderWhitelist*
BraveBrowserProcessImpl::speedreader_whitelist() {
  if (!speedreader_whitelist_) {
    speedreader_whitelist_.reset(new speedreader::SpeedreaderWhitelist(
        brave_component_updater_delegate()));
  }
  return speedreader_whitelist_.get();
}
#endif  // BUILDFLAG(ENABLE_SPEEDREADER)
