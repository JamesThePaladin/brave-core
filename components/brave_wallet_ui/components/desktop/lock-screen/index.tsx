import * as React from 'react'

import {
  StyledWrapper,
  Title,
  IconBackground,
  PageIcon,
  InputColumn,
  Input
} from '../wallet-onboarding/create-password/style'
import { NavButton } from '../../extension'
import locale from '../../../constants/locale'

export interface Props {
  action: () => void
  password: (event: any) => void
  disabled: boolean
}

function LockScreen (props: Props) {
  const { action, password, disabled } = props

  return (
    <StyledWrapper>
      <IconBackground>
        <PageIcon />
      </IconBackground>
      <Title>{locale.lockScreenTitle}</Title>
      <InputColumn>
        <Input type='password' placeholder={locale.createPasswordInput} onChange={password} />
      </InputColumn>
      <NavButton buttonType='primary' text={locale.lockScreenButton} onSubmit={action} disabled={disabled} />
    </StyledWrapper>
  )
}

export default LockScreen
