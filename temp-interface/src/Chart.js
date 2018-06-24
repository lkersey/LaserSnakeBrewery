import React, { Component } from 'react';
import axios from 'axios';
import moment from 'moment'
import {VictoryChart, VictoryTheme, VictoryLine, VictoryAxis, VictoryArea,
VictoryTooltip, VictoryVoronoiContainer} from 'victory';
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
    axios.get('http://localhost:5000/history')
    .then(res => {
      const data = res.data;
      this.setState({ inputData:data });

      const vatTemp = this.state.inputData.map(d =>
        ({x: moment(d.timestamp * 1000).format('H:mm:ss a'),
        y: d.vat_temp})
      );
      const fridgeTemp = this.state.inputData.map(d =>
        ({x: moment(d.timestamp * 1000).format('H:mm:ss a'),
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
    return this.state.chartHeight / 10;
  }

  render() {
    const padding = this.getPaddingValue()
    return (
      <div className='Chart'>
        <VictoryChart
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
            tickCount={10}
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
              data: { stroke: " e67e22 " },
            }}
            labels={ (d) => `${d.y}\u2103` }
            labelComponent={ <VictoryTooltip/> }
            data = { this.state.vatTempData }
          />

          <VictoryLine
            style={{
              data: { stroke: " 34495e " },
            }}
            labels={(d) => `${d.y}\u2103` }
            labelComponent={<VictoryTooltip/>}
            data = {this.state.fridgeTempData}
          />

        </VictoryChart>
      </div>
    )
  }
}

export default Chart;
