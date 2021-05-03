import * as React from 'react'
import {
  WalletSubViewLayout,
  OnboardingWelcome,
  OnboardingBackup,
  OnboardingRecovery,
  OnboardingVerify,
  OnboardingCreatePassword
} from '../../components/desktop'

export interface Props {
  recoveryPhrase: string[]
  action: () => void
}

function Onboarding (props: Props) {
  const { recoveryPhrase, action } = props
  const [onboardingStep, setOnboardingStep] = React.useState<number>(0)
  const [backupTerms, setBackupTerms] = React.useState<boolean>(false)
  const [backedUp, setBackedUp] = React.useState<boolean>(false)
  const [sortedPhrase, setSortedPhrase] = React.useState<string[]>([])
  const [verifyError, setVerifyError] = React.useState<boolean>(false)
  const [password, setPassword] = React.useState<string>('')
  const [confirmedPassword, setConfirmedPassword] = React.useState<string>('')

  const onRestore = () => {
    alert('Start Restore Process')
  }

  const nextStep = () => {
    if (onboardingStep === 4) {
      action()
    } else {
      setOnboardingStep(onboardingStep + 1)
    }
  }

  const checkedBox = (key: string, selected: boolean) => {
    if (key === 'backupTerms') {
      setBackupTerms(selected)
    }
    if (key === 'backedUp') {
      setBackedUp(selected)
    }
  }

  const selectWord = (word: string) => {
    const newList = [...sortedPhrase, word]
    setSortedPhrase(newList)
    setVerifyError(false)
  }

  const unSelectWord = (word: string) => {
    const newList = sortedPhrase.filter((key) => key !== word)
    setSortedPhrase(newList)
  }

  const shuffledPhrase = React.useMemo(() => {
    const array = recoveryPhrase.slice().sort()
    for (let i = array.length - 1; i > 0; i--) {
      let j = Math.floor(Math.random() * (i + 1))
      let temp = array[i]
      array[i] = array[j]
      array[j] = temp
    }
    return array
  }, [])

  const showError = () => {
    setVerifyError(true)
    setTimeout(function () { setVerifyError(false) }, 3000)
  }

  const checkPhrase = () => {
    if (sortedPhrase.length === recoveryPhrase.length && sortedPhrase.every((v, i) => v === recoveryPhrase[i])) {
      nextStep()
    } else {
      setSortedPhrase([])
      showError()
    }
  }

  const copyToClipboard = async () => {
    try {
      await navigator.clipboard.writeText(recoveryPhrase.join(' '))
    } catch (e) {
      console.log(`Could not copy address ${e.toString()}`)
    }
  }

  const inputPassword = (event: any) => {
    const value = event.target.value
    setPassword(value)
  }

  const confirmPassword = (event: any) => {
    const value = event.target.value
    setConfirmedPassword(value)
  }

  const passwordsMatch = React.useMemo(() => {
    if (password === '' || confirmedPassword === '') {
      return true
    } else {
      return password !== confirmedPassword
    }
  }, [password, confirmedPassword])

  return (
    <WalletSubViewLayout>
      {onboardingStep === 0 &&
        <OnboardingWelcome
          onRestore={onRestore}
          onSetup={nextStep}
        />
      }
      {onboardingStep === 1 &&
        <OnboardingCreatePassword
          action={nextStep}
          password={inputPassword}
          confirm={confirmPassword}
          disabled={passwordsMatch}
        />
      }
      {onboardingStep === 2 &&
        <OnboardingBackup
          action={nextStep}
          termsAction={checkedBox}
          value={backupTerms}
        />
      }
      {onboardingStep === 3 &&
        <OnboardingRecovery
          action={nextStep}
          value={backedUp}
          termsAction={checkedBox}
          recoverPhrase={recoveryPhrase}
          copy={copyToClipboard}
        />
      }
      {onboardingStep === 4 &&
        <OnboardingVerify
          action={checkPhrase}
          recoveryPhrase={shuffledPhrase}
          sortedPhrase={sortedPhrase}
          selectWord={selectWord}
          unSelectWord={unSelectWord}
          verifyError={verifyError}
        />
      }
    </WalletSubViewLayout>
  )
}

export default Onboarding
