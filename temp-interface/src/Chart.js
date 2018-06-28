import React, { Component } from 'react';
import axios from 'axios';
import moment from 'moment'
import {VictoryChart, VictoryTheme, VictoryLine, VictoryAxis, VictoryArea,
VictoryTooltip, VictoryVoronoiContainer, VictoryLegend} from 'victory';
import './Chart.css'

class Chart extends Component {
  constructor(props) {
    super(props);
    this.state = {
      chartWidth: 0,
      chartHeight: 0,
      inputData: [],
      vatTempData: [],
      fridgeTempData: []
    }
  }

  getData() {
    axios.get('https://lasersnake.duckdns.org/api/history')
    .then(res => {
      const data = res.data;
      this.setState({ inputData:data });

      const vatTemp = this.state.inputData.map(d =>
        ({x: moment(d.timestamp * 1000).format('MMM Do H:mm a'),
        y: d.vat_temp})
      );
      const fridgeTemp = this.state.inputData.map(d =>
        ({x: moment(d.timestamp * 1000).format('MMM Do H:mm a'),
        y: d.fridge_temp})
      );
      this.setState({ vatTempData:vatTemp });
      this.setState({ fridgeTempData:fridgeTemp});
    })
  }

  componentDidMount() {
    this.getData()
    this.interval = setInterval(() => this.getData(), 1000 * 60 );
    this.setState({ chartWidth: window.innerWidth });
    this.setState({ chartHeight: window.innerHeight });

    window.addEventListener('resize', this.updateDimensions.bind(this));
  }

  componentWillUnmount() {
    window.removeEventListener('resize', this.updateDimensions.bind(this));
}

  updateDimensions(event) {
    this.setState({
      chartWidth: event.target.innerWidth,
      chartHeight: event.target.innerHeight
    })
  }

  getBottomPaddingValue() {
    return this.state.chartHeight / 5;
  }

  getPaddingValue() {
    return this.state.chartHeight / 10;
  }

  render() {
    const bottomPadding = this.getBottomPaddingValue()
    const padding = this.getPaddingValue()
    return (
      <div class="Chart">
        <VictoryChart
          theme={ VictoryTheme.material }
          width= {this.state.chartWidth}
          height= {this.state.chartHeight / 1.5}
          style= {{
            parent: {}
          }}
          padding= {{
            bottom: bottomPadding,
            left: padding,
            right: padding,
            top: padding
          }}
          containerComponent={<VictoryVoronoiContainer/>}
        >

          <VictoryAxis
            tickCount={6}
            style={{
              tickLabels: {
                padding: 50, angle: -45, fontSize: '14px'
              }
            }}
          />

          <VictoryAxis dependentAxis
            style={{
              tickLabels: {
                fontSize: '14px'
              }
            }}
          />

          <VictoryLine
            style={{
              data: {
                stroke: "#964100",
                strokeWidth: 4
             },
            }}
            labels={ (d) => `${d.y}\u2103` }
            labelComponent={ <VictoryTooltip/> }
            data = { this.state.vatTempData }
          />

          <VictoryLine
            style={{
              data: {
                stroke: "#758a5a",
                strokeWidth: 2
              },
            }}
            labels={(d) => `${d.y}\u2103` }
            labelComponent={ <VictoryTooltip/> }
            data = {this.state.fridgeTempData}
          />

        </VictoryChart>

      </div>
    )
  }
}

export default Chart;
