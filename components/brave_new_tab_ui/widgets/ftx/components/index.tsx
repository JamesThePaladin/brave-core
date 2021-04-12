// Copyright (c) 2021 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { ThemeProvider, ThemeConsumer } from 'styled-components'
import { CaratLeftIcon } from 'brave-ui/components/icons'
import Loading from '../../../components/loading'
import createWidget from '../../../components/default/widget/index'
import { StyledTitleTab } from '../../../components/default/widgetTitleTab'
import {
  ShowIcon,
  HideIcon
} from '../../../components/default/exchangeWidget/shared-assets'
import {
  BackArrow,
  BasicBox,
  Box,
  WidgetIcon,
  FlexItem,
  Header,
  List,
  ListItem,
  OptionButton,
  StyledTitle,
  StyledTitleText,
  Text,
  WidgetWrapper,
  PlainAnchor,
  Balance,
  BlurIcon
} from '../../shared/styles'
import { currencyNames } from '../../shared/data'
import getFormattedPrice from '../../shared/getFormattedPrice'
import * as FTXActions from '../ftx_actions'
import { FTXState, ViewType } from '../ftx_state'
import ftxLogo from './ftx-logo.png'
import customizeTheme from './theme'
import Convert from './convert'
import IconAsset from '../../shared/iconAsset'
import AssetDetail from './assetDetail'
import PreOptIn from './preOptIn'

// Utils
interface State {
  hideBalance: boolean
}

interface Props {
  ftx: FTXState
  actions: typeof FTXActions
  widgetTitle: string
  showContent: boolean
  stackPosition: number
  onShowContent: () => void
}

class FTX extends React.PureComponent<Props, State> {
  constructor (props: Props) {
    super(props)
    this.state = {
      hideBalance: true
    }
  }

  componentDidMount () {
    if (this.props.showContent && !this.props.ftx.hasInitialized) {
      this.props.actions.initialize()
    }
  }

  componentDidUpdate (prevProps: Props) {
    const isNewlyShown = this.props.showContent && !prevProps.showContent
    if (isNewlyShown && !this.props.ftx.hasInitialized) {
      this.props.actions.initialize()
    }
  }

  handleAssetDetailClick = async (symbol: string) => {
    this.props.actions.showAssetDetail({ symbol })
  }

  renderMarkets () {
    return (
      <>
        <List $mt={10}>
          {this.props.ftx.marketData.map(market => {
            const { symbol, price, percentChangeDay } = market
            const currencyName = currencyNames[symbol]
            return (
              <ListItem key={symbol} isFlex={true} onClick={this.handleAssetDetailClick.bind(this, symbol)} $height={48}>
                <FlexItem $pl={5} $pr={5}>
                  <IconAsset iconKey={symbol.toLowerCase()} />
                </FlexItem>
                <FlexItem>
                  <Text>{symbol}</Text>
                  {currencyName &&
                  <Text small={true} textColor='light'>{currencyNames[symbol]}</Text>
                  }
                </FlexItem>
                <FlexItem textAlign='right' flex={1}>
                  <Text>{getFormattedPrice(price)}</Text>
                  <Text textColor={percentChangeDay > 0 ? 'green' : 'red'}>{percentChangeDay}%</Text>
                </FlexItem>
              </ListItem>
            )
          })}
        </List>
        <Text $mt={13} center={true}>More markets on <PlainAnchor href='#'>ftx.com</PlainAnchor></Text>
      </>
    )
  }

  // TODO: remove
  toggleBalanceVisibility = () => {
    this.setState({
      hideBalance: !this.state.hideBalance
    })
  }

  renderSummary () {
    const { hideBalance } = this.state
    const total = this.props.ftx.balanceTotal
    return (
      <Box $mt={10}>
        <FlexItem isFlex={true} $p={15} hasPadding={true} >
          {total !== null &&
          <FlexItem>
            <Balance hideBalance={hideBalance}>
              <Text lineHeight={1.15} $fontSize={21}>{getFormattedPrice(total)}</Text>
            </Balance>
          </FlexItem>
          }
          <FlexItem>
            <BlurIcon onClick={this.toggleBalanceVisibility}>
              {
                hideBalance
                ? <ShowIcon />
                : <HideIcon />
              }
            </BlurIcon>
          </FlexItem>
        </FlexItem>
        <List hasBorder={false}>
          {Object.keys(this.props.ftx.balances).map(currencyKey => {
            const balance = this.props.ftx.balances[currencyKey]
            return (
              <ListItem key={currencyKey} isFlex={true} $height={40}>
                <FlexItem $pl={5} $pr={5}>
                  <IconAsset iconKey={currencyKey.toLowerCase()} size={18} />
                </FlexItem>
                <FlexItem>
                  <Text>{currencyKey}</Text>
                </FlexItem>
                <FlexItem textAlign='right' flex={1}>
                  <Balance hideBalance={hideBalance}>
                    <Text lineHeight={1.15}>{balance}</Text>
                  </Balance>
                </FlexItem>
              </ListItem>
            )
          })}
        </List>
      </Box>
    )
  }

  renderView () {
    const selectedAsset = this.props.ftx.assetDetail?.currencyName
    const { currentView } = this.props.ftx
    if (selectedAsset) {
      return <AssetDetail ftx={this.props.ftx} actions={this.props.actions} />
    } else if (currentView === ViewType.Convert) {
      return <Convert ftx={this.props.ftx} actions={this.props.actions} />
    } else if (currentView === ViewType.Summary) {
      return this.renderSummary()
    } else {
      return this.renderMarkets()
    }
  }

  setView = (view: ViewType) => {
    this.props.actions.openView(view)
  }

  renderContent () {
    const { currentView, isConnected, hasInitialized } = this.props.ftx
    if (!hasInitialized) {
      return <BasicBox $height={250}><Loading /></BasicBox>
    }
    if (!isConnected && this.props.ftx.currentView === ViewType.OptIn) {
      return <PreOptIn ftx={this.props.ftx} actions={this.props.actions} />
    }
    return (
      <>
        {isConnected &&
        <BasicBox isFlex={true} $mb={10} $gap={10} justify='start'>
          <OptionButton isSelected={currentView === ViewType.Markets} onClick={this.setView.bind(null, ViewType.Markets)}>Markets</OptionButton>
          <OptionButton isSelected={currentView === ViewType.Convert} onClick={this.setView.bind(null, ViewType.Convert)}>Convert</OptionButton>
          <OptionButton isSelected={currentView === ViewType.Summary} onClick={this.setView.bind(null, ViewType.Summary)}>Summary</OptionButton>
        </BasicBox>
        }
        {this.renderView()}
      </>
    )
  }

  renderTitle () {
    const selectedAsset = this.props.ftx.assetDetail?.currencyName
    const { showContent, widgetTitle } = this.props
    // Only show back arrow to go back to opt-in view
    const shouldShowBackArrow = !selectedAsset &&
      this.props.ftx.currentView !== ViewType.OptIn &&
      !this.props.ftx.isConnected

    return (
      <Header showContent={showContent}>
        <StyledTitle>
          <WidgetIcon>
            <img src={ftxLogo} alt='FTX logo'/>
          </WidgetIcon>
          <StyledTitleText>
            {widgetTitle}
          </StyledTitleText>
          {shouldShowBackArrow &&
            <BackArrow marketView={true}>
              <CaratLeftIcon
                onClick={this.props.actions.preOptInViewMarkets.bind(undefined, { hide: true })}
              />
            </BackArrow>
          }
        </StyledTitle>
      </Header>
    )
  }

  renderTitleTab () {
    const { onShowContent, stackPosition } = this.props

    return (
      <StyledTitleTab onClick={onShowContent} stackPosition={stackPosition}>
        {this.renderTitle()}
      </StyledTitleTab>
    )
  }

  render () {
    const { showContent } = this.props

    if (!showContent) {
      return this.renderTitleTab()
    }

    return (
      <ThemeConsumer>
      {theme =>
        <ThemeProvider theme={customizeTheme(theme)}>
          <WidgetWrapper tabIndex={0}>
            {this.renderTitle()}
            {this.renderContent()}
          </WidgetWrapper>
        </ThemeProvider>
      }
      </ThemeConsumer>
    )
  }
}

export const FTXWidget = createWidget(FTX)
