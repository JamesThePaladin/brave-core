import * as React from 'react'

import {
  StyledWrapper,
  Title,
  Description,
  TermsRow,
  CopyButton,
  WarningBox,
  WarningText,
  DisclaimerText,
  DisclaimerColumn,
  AlertIcon,
  RecoveryBubble,
  RecoveryBubbleText,
  RecoveryPhraseContainer
} from './style'
import { NavButton } from '../../../extension'
import locale from '../../../../constants/locale'
import { Checkbox } from 'brave-ui'

export interface Props {
  action: () => void
  value: boolean
  termsAction: (key: string, selected: boolean) => void
  recoverPhrase: string[]
  copy: () => void
}

function OnboardingBackup (props: Props) {
  const { action, value, termsAction, recoverPhrase, copy } = props

  return (
    <StyledWrapper>
      <Title>{locale.recoveryTitle}</Title>
      <Description>{locale.recoveryDescription}</Description>
      <WarningBox>
        <AlertIcon />
        <DisclaimerColumn>
          <DisclaimerText><WarningText>{locale.recoveryWarning1} </WarningText>{locale.recoveryWarning2}</DisclaimerText>
          <DisclaimerText>{locale.recoveryWarning3}</DisclaimerText>
        </DisclaimerColumn>
      </WarningBox>
      <RecoveryPhraseContainer>
        {recoverPhrase.map((word) =>
          <RecoveryBubble key={word}>
            <RecoveryBubbleText>{recoverPhrase.indexOf(word) + 1}. {word}</RecoveryBubbleText>
          </RecoveryBubble>
        )}
      </RecoveryPhraseContainer>
      <CopyButton onClick={copy}>{locale.buttonCopy}</CopyButton>
      <TermsRow>
        <Checkbox value={{ backedUp: value }} onChange={termsAction}>
          <div data-key='backedUp'>{locale.recoveryTerms}</div>
        </Checkbox>
      </TermsRow>
      <NavButton disabled={!value} buttonType='primary' text={locale.buttonContinue} onSubmit={action} />
    </StyledWrapper>
  )
}

export default OnboardingBackup
