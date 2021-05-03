import * as React from 'react'
import { WalletWidgetStandIn } from './style'
import {
  SideNav,
  WalletPageLayout,
  WalletSubViewLayout,
  CryptoView,
  LockScreen
} from '../components/desktop'
import {
  NavTypes,
  NavObjectType
} from '../constants/types'
import Onboarding from './screens/onboarding'
import { LinkedAccountsOptions, NavOptions, StaticOptions } from '../options/side-nav-options'
import BuySendSwap from '../components/buy-send-swap'
import { recoveryPhrase } from './mock-data/user-accounts'
export default {
  title: 'Wallet/Desktop',
  argTypes: {
    onboarding: { control: { type: 'boolean', onboard: false } },
    locked: { control: { type: 'boolean', onboard: false } }
  }
}

export const _DesktopWalletConcept = (args: { onboarding: boolean, locked: boolean }) => {
  const { onboarding, locked } = args
  const [view, setView] = React.useState<NavTypes>('crypto')
  const [linkedAccounts] = React.useState<NavObjectType[]>(LinkedAccountsOptions)
  const [needsOnboarding, setNeedsOnboarding] = React.useState<boolean>(onboarding)
  const [walletLocked, setWalletLocked] = React.useState<boolean>(locked)
  const [inputValue, setInputValue] = React.useState<string>('')

  // In the future these will be actual paths
  // for example wallet/rewards
  const navigateTo = (path: NavTypes) => {
    setView(path)
  }

  const completeWalletSetup = () => {
    setNeedsOnboarding(false)
  }

  const unlockWallet = () => {
    setWalletLocked(false)
  }

  const inputPassword = (event: any) => {
    const value = event.target.value
    setInputValue(value)
  }

  return (
    <WalletPageLayout>
      <SideNav
        navList={NavOptions}
        staticList={StaticOptions}
        selectedButton={view}
        onSubmit={navigateTo}
        linkedAccountsList={linkedAccounts}
      />
      {needsOnboarding ?
        (
          <Onboarding recoveryPhrase={recoveryPhrase} action={completeWalletSetup} />
        ) : (
          <WalletSubViewLayout>
            {view === 'crypto' ? (
              <>
                {walletLocked ? (
                  <LockScreen action={unlockWallet} disabled={inputValue === ''} password={inputPassword} />
                ) : (
                  <CryptoView />
                )}
              </>
            ) : (
              <div style={{ flex: 1, alignItems: 'center', justifyContent: 'center' }}>
                <h2>{view} view</h2>
              </div>
            )}
          </WalletSubViewLayout>
        )}

      <WalletWidgetStandIn>
        {!needsOnboarding && !walletLocked &&
          <BuySendSwap />
        }
      </WalletWidgetStandIn>
    </WalletPageLayout>
  )
}

_DesktopWalletConcept.args = {
  onboarding: true
}

_DesktopWalletConcept.story = {
  name: 'Concept'
}
