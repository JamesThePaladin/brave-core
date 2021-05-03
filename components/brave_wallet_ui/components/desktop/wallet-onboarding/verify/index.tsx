import * as React from 'react'

import {
  StyledWrapper,
  Title,
  Description,
  RecoveryBubble,
  RecoveryBubbleText,
  RecoveryPhraseContainer,
  SelectedPhraseContainer,
  SelectedBubble,
  SelectedBubbleText,
  ErrorText,
  ErrorContainer
} from './style'
import { NavButton } from '../../../extension'
import locale from '../../../../constants/locale'

export interface Props {
  action: () => void
  recoveryPhrase: string[]
  sortedPhrase: string[]
  selectWord: (word: string) => void
  unSelectWord: (word: string) => void
  verifyError: boolean
}

function OnboardingVerify (props: Props) {
  const { action, recoveryPhrase, selectWord, unSelectWord, sortedPhrase, verifyError } = props

  const addWord = (word: string) => () => {
    selectWord(word)
  }

  const removeWord = (word: string) => () => {
    unSelectWord(word)
  }

  return (
    <StyledWrapper>
      <Title>{locale.verifyRecoveryTitle}</Title>
      <Description>{locale.verifyRecoveryDescription}</Description>
      <SelectedPhraseContainer error={verifyError}>
        {sortedPhrase.map((word) =>
          <SelectedBubble
            key={word}
            onClick={removeWord(word)}
          >
            <SelectedBubbleText>{sortedPhrase.indexOf(word) + 1}. {word}</SelectedBubbleText>
          </SelectedBubble>
        )}
        {verifyError &&
          <ErrorContainer>
            <ErrorText>{locale.verifyError}</ErrorText>
          </ErrorContainer>
        }
      </SelectedPhraseContainer>
      <RecoveryPhraseContainer>
        {recoveryPhrase.map((word) =>
          <RecoveryBubble
            key={word}
            onClick={addWord(word)}
            disabled={sortedPhrase.includes(word)}
            isSelected={sortedPhrase.includes(word)}
          >
            <RecoveryBubbleText isSelected={sortedPhrase.includes(word)}>{word}</RecoveryBubbleText>
          </RecoveryBubble>
        )}
      </RecoveryPhraseContainer>
      <NavButton disabled={sortedPhrase.length !== 12} buttonType='primary' text={locale.buttonVerify} onSubmit={action} />
    </StyledWrapper>
  )
}

export default OnboardingVerify
