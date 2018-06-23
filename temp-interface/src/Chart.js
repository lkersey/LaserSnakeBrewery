import React, { Component } from 'react';
import axios from 'axios';
import moment from 'moment'
import {VictoryChart, VictoryTheme, VictoryLine, VictoryAxis} from 'victory';
import './Chart.css'

class Chart extends Component {
  constructor(props) {
    super(props);
    this.state = {
      inputData:[],
      vatTempData:[]
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
      this.setState({ vatTempData:vatTemp });
    })
  }

  componentDidMount() {
    this.getData()
    this.interval = setInterval(() => this.getData(), 1000);
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
        }}>

          <VictoryLine style={{
            data: { stroke: "c43a31" },
            parent: { border: "1px solid #ccc" }
          }}
          data = {this.state.vatTempData}
          />

          <VictoryAxis
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

        </VictoryChart>
      </div>
    )
  }
}

export default Chart;
