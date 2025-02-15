// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import { MiddlewareAPI, Dispatch, AnyAction } from 'redux'
import AsyncActionHandler from '../../../common/AsyncActionHandler'
import * as Actions from '../actions/wallet_page_actions'
import { PageState, WalletPageState } from '../../constants/types'

const handler = new AsyncActionHandler()

function getPageState (store: MiddlewareAPI<Dispatch<AnyAction>, any>): PageState {
  return (store.getState() as WalletPageState).page
}

handler.on(Actions.initialize.getType(), async (store) => {
  const state = getPageState(store)
  // Sanity check we only initialize once
  if (state.hasInitialized) {
    return
  }
  // TODO: Fetch any data we need for initial display, instead of fake wait.
  await new Promise(resolve => setTimeout(resolve, 400))
  store.dispatch(Actions.initialized({ isConnected: true }))
  return
})

export default handler.middleware
