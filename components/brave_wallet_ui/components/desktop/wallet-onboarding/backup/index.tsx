import * as React from 'react'

import {
  StyledWrapper,
  Title,
  Description,
  IconBackground,
  PageIcon,
  TermsRow
} from './style'
import { NavButton } from '../../../extension'
import locale from '../../../../constants/locale'
import { Checkbox } from 'brave-ui'

export interface Props {
  action: () => void
  value: boolean
  termsAction: (key: string, selected: boolean) => void
}

function OnboardingRecovery (props: Props) {
  const { action, value, termsAction } = props

  return (
    <StyledWrapper>
      <IconBackground>
        <PageIcon />
      </IconBackground>
      <Title>{locale.backupIntroTitle}</Title>
      <Description>{locale.backupIntroDescription}</Description>
      <TermsRow>
        <Checkbox value={{ backupTerms: value }} onChange={termsAction}>
          <div data-key='backupTerms'>{locale.backupIntroTerms}</div>
        </Checkbox>
      </TermsRow>
      <NavButton disabled={!value} buttonType='primary' text={locale.buttonContinue} onSubmit={action} />
    </StyledWrapper>
  )
}

export default OnboardingRecovery
