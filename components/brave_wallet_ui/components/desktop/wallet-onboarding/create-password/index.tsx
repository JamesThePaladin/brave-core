import * as React from 'react'

import {
  StyledWrapper,
  Title,
  IconBackground,
  PageIcon,
  InputColumn,
  Input
} from './style'
import { NavButton } from '../../../extension'
import locale from '../../../../constants/locale'

export interface Props {
  action: () => void
  password: (event: any) => void
  confirm: (event: any) => void
  disabled: boolean
}

function OnboardingCreatePassword (props: Props) {
  const { action, password, confirm, disabled } = props

  return (
    <StyledWrapper>
      <IconBackground>
        <PageIcon />
      </IconBackground>
      <Title>{locale.createPasswordTitle}</Title>
      <InputColumn>
        <Input type='password' placeholder={locale.createPasswordInput} onChange={password} />
        <Input type='password' placeholder={locale.createPasswordInput2} onChange={confirm} />
      </InputColumn>
      <NavButton buttonType='primary' text={locale.buttonContinue} onSubmit={action} disabled={disabled} />
    </StyledWrapper>
  )
}

export default OnboardingCreatePassword
