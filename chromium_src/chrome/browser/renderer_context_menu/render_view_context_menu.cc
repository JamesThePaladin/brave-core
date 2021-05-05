// Copyright 2018 The Brave Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/renderer_context_menu/render_view_context_menu.h"

#include "base/strings/utf_string_conversions.h"
#include "brave/browser/autocomplete/brave_autocomplete_scheme_classifier.h"
#include "brave/browser/profiles/profile_util.h"
#include "brave/browser/renderer_context_menu/brave_spelling_options_submenu_observer.h"
#include "brave/browser/translate/buildflags/buildflags.h"
#include "brave/components/ipfs/buildflags/buildflags.h"
#include "brave/components/tor/buildflags/buildflags.h"
#include "brave/grit/brave_theme_resources.h"
#include "chrome/browser/autocomplete/chrome_autocomplete_provider_client.h"
#include "components/omnibox/browser/autocomplete_classifier.h"
#include "components/omnibox/browser/autocomplete_controller.h"
#include "ui/base/models/menu_separator_types.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/paint_vector_icon.h"

#if BUILDFLAG(ENABLE_TOR)
#include "brave/browser/tor/tor_profile_manager.h"
#include "brave/browser/tor/tor_profile_service_factory.h"
#endif

#if BUILDFLAG(IPFS_ENABLED)
#include "brave/browser/ipfs/import/ipfs_import_controller.h"
#include "brave/browser/ipfs/ipfs_helper.h"
#include "brave/browser/ipfs/ipfs_service_factory.h"
#include "brave/browser/ipfs/ipfs_tab_helper.h"
#include "brave/components/ipfs/ipfs_constants.h"
#include "brave/components/ipfs/ipfs_service.h"
#include "brave/components/ipfs/ipfs_utils.h"
#include "brave/components/ipfs/keys/ipns_keys_manager.h"
#endif
// Our .h file creates a masquerade for RenderViewContextMenu.  Switch
// back to the Chromium one for the Chromium implementation.
#undef RenderViewContextMenu
#define RenderViewContextMenu RenderViewContextMenu_Chromium

namespace {

GURL GetSelectionNavigationURL(Profile* profile, const std::u16string& text) {
  AutocompleteMatch match;
  AutocompleteClassifier classifier(
      std::make_unique<AutocompleteController>(
          std::make_unique<ChromeAutocompleteProviderClient>(profile),
          AutocompleteClassifier::DefaultOmniboxProviders()),
      std::make_unique<BraveAutocompleteSchemeClassifier>(profile));
  classifier.Classify(text, false, false,
                      metrics::OmniboxEventProto::INVALID_SPEC, &match, NULL);
  classifier.Shutdown();
  return match.destination_url;
}

base::OnceCallback<void(BraveRenderViewContextMenu*)>*
BraveGetMenuShownCallback() {
  static base::NoDestructor<
      base::OnceCallback<void(BraveRenderViewContextMenu*)>>
      callback;
  return callback.get();
}

}  // namespace

void RenderViewContextMenu::RegisterMenuShownCallbackForTesting(
    base::OnceCallback<void(BraveRenderViewContextMenu*)> cb) {
  *BraveGetMenuShownCallback() = std::move(cb);
}

#define BRAVE_APPEND_SEARCH_PROVIDER \
  if (GetProfile()->IsOffTheRecord()) { \
    selection_navigation_url_ = \
        GetSelectionNavigationURL(GetProfile(), params_.selection_text); \
    if (!selection_navigation_url_.is_valid()) \
      return; \
  }

// Use our subclass to initialize SpellingOptionsSubMenuObserver.
#define SpellingOptionsSubMenuObserver BraveSpellingOptionsSubMenuObserver
#define RegisterMenuShownCallbackForTesting \
  RegisterMenuShownCallbackForTesting_unused

#include "../../../../../chrome/browser/renderer_context_menu/render_view_context_menu.cc"

#undef SpellingOptionsSubMenuObserver
#undef RegisterMenuShownCallbackForTesting

// Make it clear which class we mean here.
#undef RenderViewContextMenu
#undef BRAVE_APPEND_SEARCH_PROVIDER

BraveRenderViewContextMenu::BraveRenderViewContextMenu(
    content::RenderFrameHost* render_frame_host,
    const content::ContextMenuParams& params)
    : RenderViewContextMenu_Chromium(render_frame_host, params)
#if BUILDFLAG(IPFS_ENABLED)
      ,
      ipfs_submenu_model_(this)
#endif
{
}

BraveRenderViewContextMenu::~BraveRenderViewContextMenu() {}

bool BraveRenderViewContextMenu::IsCommandIdEnabled(int id) const {
  if (id >= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_START &&
      id <= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_END) {
    return ipfs::IpnsKeysAvailable(browser_context_);
  }

  switch (id) {
#if BUILDFLAG(IPFS_ENABLED)
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS_PAGE:
    case IDC_CONTENT_CONTEXT_IMPORT_IMAGE_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_VIDEO_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_AUDIO_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_LINK_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_SELECTED_TEXT_IPFS:
      return IsIPFSCommandIdEnabled(id);
#endif
    case IDC_CONTENT_CONTEXT_OPENLINKTOR:
#if BUILDFLAG(ENABLE_TOR)
      if (brave::IsTorDisabledForProfile(GetProfile()))
        return false;

      return params_.link_url.is_valid() &&
             IsURLAllowedInIncognito(params_.link_url, browser_context_) &&
             !GetProfile()->IsTor();
#else
      return false;
#endif
    default:
      return RenderViewContextMenu_Chromium::IsCommandIdEnabled(id);
  }
}
#if BUILDFLAG(IPFS_ENABLED)
void BraveRenderViewContextMenu::ExecuteIPFSCommand(int id,
                                                    const std::string& key) {
  ipfs::IPFSTabHelper* helper =
      ipfs::IPFSTabHelper::FromWebContents(source_web_contents_);
  if (!helper)
    return;
  auto* controller = helper->GetImportController();
  if (!controller)
    return;
  switch (id) {
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS_PAGE:
      helper->ImportCurrentPageToIpfs(key);
      break;
    case IDC_CONTENT_CONTEXT_IMPORT_IMAGE_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_VIDEO_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_AUDIO_IPFS:
      controller->ImportLinkToIpfs(params_.src_url, key);
      break;
    case IDC_CONTENT_CONTEXT_IMPORT_LINK_IPFS:
      controller->ImportLinkToIpfs(params_.link_url, key);
      break;
    case IDC_CONTENT_CONTEXT_IMPORT_SELECTED_TEXT_IPFS:
      controller->ImportTextToIpfs(base::UTF16ToUTF8(params_.selection_text),
                                   key);
      break;
  }
}

int BraveRenderViewContextMenu::GetSelectedIPFSCommandId(int id) const {
  for (const auto& it : ipns_submenu_models_) {
    auto index = it.second->GetIndexOfCommandId(id);
    if (index == -1)
      continue;
    return it.first;
  }
  return -1;
}
#endif

void BraveRenderViewContextMenu::ExecuteCommand(int id, int event_flags) {
#if BUILDFLAG(IPFS_ENABLED)
  if (id >= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_START &&
      id <= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_END) {
    int ipfs_command = GetSelectedIPFSCommandId(id);
    if (ipfs_command == -1)
      return;
    auto* submenu = ipns_submenu_models_[ipfs_command].get();
    auto command_index = submenu->GetIndexOfCommandId(id);
    if (command_index == -1)
      return;
    auto label = base::UTF16ToUTF8(submenu->GetLabelAt(command_index));
    auto key_name = (command_index > 0) ? label : std::string();
    ExecuteIPFSCommand(ipfs_command, key_name);
    return;
  }
#endif
  switch (id) {
#if BUILDFLAG(IPFS_ENABLED)
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS_PAGE:
    case IDC_CONTENT_CONTEXT_IMPORT_IMAGE_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_VIDEO_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_AUDIO_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_LINK_IPFS:
    case IDC_CONTENT_CONTEXT_IMPORT_SELECTED_TEXT_IPFS:
      ExecuteIPFSCommand(id, std::string());
      break;
#endif
    case IDC_CONTENT_CONTEXT_OPENLINKTOR:
      TorProfileManager::SwitchToTorProfile(
          GetProfile(),
          base::Bind(
              OnProfileCreated, params_.link_url,
              content::Referrer(
                  GURL(), network::mojom::ReferrerPolicy::kStrictOrigin)));
      break;
    default:
      RenderViewContextMenu_Chromium::ExecuteCommand(id, event_flags);
  }
}

void BraveRenderViewContextMenu::AddSpellCheckServiceItem(bool is_checked) {
  // Call our implementation, not the one in the base class.
  // Assumption:
  // Use of spelling service is disabled in Brave profile preferences.
  DCHECK(!GetProfile()->GetPrefs()->GetBoolean(
      spellcheck::prefs::kSpellCheckUseSpellingService));
  AddSpellCheckServiceItem(&menu_model_, is_checked);
}

// static
void BraveRenderViewContextMenu::AddSpellCheckServiceItem(
    ui::SimpleMenuModel* menu,
    bool is_checked) {
  // Suppress adding "Spellcheck->Ask Brave for suggestions" item.
}

#if BUILDFLAG(IPFS_ENABLED)
bool BraveRenderViewContextMenu::IsIPFSCommandIdEnabled(int command) const {
  if (!ipfs::IsIpfsMenuEnabled(browser_context_))
    return false;

  if (command >= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_START &&
      command <= IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_END) {
    return ipfs::IpnsKeysAvailable(browser_context_);
  }

  switch (command) {
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS:
      return true;
    case IDC_CONTENT_CONTEXT_IMPORT_IPFS_PAGE:
      return source_web_contents_->GetURL().SchemeIsHTTPOrHTTPS() &&
             source_web_contents_->IsSavable();
    case IDC_CONTENT_CONTEXT_IMPORT_IMAGE_IPFS:
      return params_.has_image_contents;
    case IDC_CONTENT_CONTEXT_IMPORT_VIDEO_IPFS:
      return content_type_->SupportsGroup(
          ContextMenuContentType::ITEM_GROUP_MEDIA_VIDEO);
    case IDC_CONTENT_CONTEXT_IMPORT_AUDIO_IPFS:
      return content_type_->SupportsGroup(
          ContextMenuContentType::ITEM_GROUP_MEDIA_AUDIO);
    case IDC_CONTENT_CONTEXT_IMPORT_LINK_IPFS:
      return !params_.link_url.is_empty();
    case IDC_CONTENT_CONTEXT_IMPORT_SELECTED_TEXT_IPFS:
      return !params_.selection_text.empty() &&
             params_.media_type == ContextMenuDataMediaType::kNone;
    default:
      NOTREACHED();
  }
  return false;
}

void BraveRenderViewContextMenu::SeIpfsIconAt(int index) {
  auto& bundle = ui::ResourceBundle::GetSharedInstance();
  const auto& ipfs_logo = *bundle.GetImageSkiaNamed(IDR_BRAVE_IPFS_LOGO);
  ui::ImageModel model = ui::ImageModel::FromImageSkia(ipfs_logo);
  menu_model_.SetIcon(index, model);
}

int BraveRenderViewContextMenu::AddIpfsImportMenuItem(int action_command_id,
                                                      int string_id,
                                                      int keys_command_id) {
  if (ipfs::IpnsKeysAvailable(browser_context_)) {
    DCHECK(!ipns_submenu_models_.count(action_command_id));
    ipns_submenu_models_[action_command_id] =
        std::make_unique<ui::SimpleMenuModel>(this);
    auto* keys_submenu = ipns_submenu_models_[action_command_id].get();
    DCHECK(keys_submenu);
    auto* keys_manager = ipfs::GetIpnsKeysManager(browser_context_);
    DCHECK(keys_manager);
    auto items_added =
        ipfs::AddIpnsKeysToSubMenu(keys_submenu, keys_manager, keys_command_id);
    ipfs_submenu_model_.AddSubMenuWithStringId(action_command_id, string_id,
                                               keys_submenu);
    return items_added;
  }
  ipfs_submenu_model_.AddItemWithStringId(action_command_id, string_id);
  return 0;
}

void BraveRenderViewContextMenu::BuildIPFSMenu() {
  if (!ipfs::IsIpfsMenuEnabled(browser_context_))
    return;
  int index =
      menu_model_.GetIndexOfCommandId(IDC_CONTENT_CONTEXT_INSPECTELEMENT);
  if (index == -1)
    return;

  int keys_command_index = IDC_CONTENT_CONTEXT_IMPORT_IPNS_KEYS_START;
  if (!params_.selection_text.empty() &&
      params_.media_type == ContextMenuDataMediaType::kNone) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_SELECTED_TEXT_IPFS,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_SELECTED_TEXT, keys_command_index);
  }
  if (source_web_contents_->GetURL().SchemeIsHTTPOrHTTPS()) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_IPFS_PAGE,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_PAGE, keys_command_index);
  }
  if (params_.has_image_contents) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_IMAGE_IPFS,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_IMAGE, keys_command_index);
  }
  if (content_type_->SupportsGroup(
          ContextMenuContentType::ITEM_GROUP_MEDIA_VIDEO)) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_VIDEO_IPFS,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_VIDEO, keys_command_index);
  }
  if (content_type_->SupportsGroup(
          ContextMenuContentType::ITEM_GROUP_MEDIA_AUDIO)) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_AUDIO_IPFS,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_AUDIO, keys_command_index);
  }
  if (!params_.link_url.is_empty()) {
    keys_command_index += AddIpfsImportMenuItem(
        IDC_CONTENT_CONTEXT_IMPORT_LINK_IPFS,
        IDS_CONTENT_CONTEXT_IMPORT_IPFS_LINK, keys_command_index);
  }
  if (!ipfs_submenu_model_.GetItemCount())
    return;
  menu_model_.InsertSeparatorAt(index, ui::MenuSeparatorType::NORMAL_SEPARATOR);
  menu_model_.InsertSubMenuWithStringIdAt(
      index, IDC_CONTENT_CONTEXT_IMPORT_IPFS, IDS_CONTENT_CONTEXT_IMPORT_IPFS,
      &ipfs_submenu_model_);
  SeIpfsIconAt(index);
}
#endif

void BraveRenderViewContextMenu::InitMenu() {
  RenderViewContextMenu_Chromium::InitMenu();

#if BUILDFLAG(ENABLE_TOR) || !BUILDFLAG(ENABLE_BRAVE_TRANSLATE_GO)
  int index = -1;
#endif
#if BUILDFLAG(ENABLE_TOR)
  // Add Open Link with Tor
  if (!TorProfileServiceFactory::IsTorDisabled() &&
      !params_.link_url.is_empty()) {
    const Browser* browser = GetBrowser();
    const bool is_app = browser && browser->is_type_app();

    index = menu_model_.GetIndexOfCommandId(
        IDC_CONTENT_CONTEXT_OPENLINKOFFTHERECORD);
    DCHECK_NE(index, -1);

    menu_model_.InsertItemWithStringIdAt(
        index + 1,
        IDC_CONTENT_CONTEXT_OPENLINKTOR,
        is_app ? IDS_CONTENT_CONTEXT_OPENLINKTOR_INAPP
               : IDS_CONTENT_CONTEXT_OPENLINKTOR);
  }
#endif

#if BUILDFLAG(IPFS_ENABLED)
  BuildIPFSMenu();
#endif

  // Only show the translate item when go-translate is enabled.
#if !BUILDFLAG(ENABLE_BRAVE_TRANSLATE_GO)
  index = menu_model_.GetIndexOfCommandId(IDC_CONTENT_CONTEXT_TRANSLATE);
  if (index != -1)
    menu_model_.RemoveItemAt(index);
#endif
}
