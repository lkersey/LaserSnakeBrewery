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

  getPaddingValue() {
    return this.state.chartHeight / 5;
  }

  render() {
    const padding = this.getPaddingValue()
    return (
      <div>
        <VictoryChart className='Chart'
          theme={ VictoryTheme.material }
          width= {this.state.chartWidth}
          height= {this.state.chartHeight / 2}
          style= {{
            parent: {}
          }}
          padding= {{
            bottom: padding,
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
                padding: 30, angle: -45, fontSize: '14px'
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
                stroke: "b57400 ",
                strokeWidth: 4
              },
            }}
            labels={(d) => `${d.y}\u2103` }
            labelComponent={<VictoryTooltip/>}
            data = {this.state.fridgeTempData}
          />

          <VictoryLine
            style={{
              data: {
                stroke: " 964100 ",
                strokeWidth: 4
             },
            }}
            labels={ (d) => `${d.y}\u2103` }
            labelComponent={ <VictoryTooltip/> }
            data = { this.state.vatTempData }
          />

          <VictoryLegend x={125} y={50}
            orientation="vertical"
    gutter={20}
    style={{ border: { stroke: "black" }, title: {fontSize: 20 } }}
    data={[
      { name: "One", symbol: { fill: "tomato", type: "star" } },
      { name: "Two", symbol: { fill: "orange" } },
      { name: "Three", symbol: { fill: "gold" } }
    ]}
  />

        </VictoryChart>
      </div>
    )
  }
}

export default Chart;
