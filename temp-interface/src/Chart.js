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
      inputData:[],
      vatTempData:[],
      fridgeTempData:[]
    }
  }

  getData() {
    axios.get('http://localhost:5000/status')
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
    this.interval = setInterval(() => this.getData(), 1000 * 60);
  }

  render() {
    return (
      <div>
      {this.state.vatTempData.length}
        <VictoryChart theme={ VictoryTheme.material }
        style= {{
          parent: { maxWidth: "60%"}
        }}
        padding= {{
          bottom: 100, left: 100, right: 50, top: 50
        }}
        containerComponent={<VictoryVoronoiContainer/>}
        >

        <VictoryAxis
        tickCount={10}
        style={{
          tickLabels: {
            padding: 20, angle: -45, fontSize: '8px'
          }
        }} />

        <VictoryAxis dependentAxis
        style={{
          tickLabels: {
            fontSize: '8px'
          }
        }} />

          <VictoryLine
            style={{
              data: { stroke: "c43a71" },
            }}
            labels={(d) => `${d.y}\u2103` }
            labelComponent={<VictoryTooltip/>}
            data = {this.state.vatTempData}
          />

          <VictoryLine
            style={{
              data: { stroke: "aaaaaa" },
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
