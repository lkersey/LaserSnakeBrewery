import React, { Component } from 'react';
import axios from 'axios';
import {VictoryChart, VictoryTheme, VictoryLine} from 'victory';
import './Chart.css'

class Chart extends Component {
  constructor(props) {
    super(props);
    this.state = {
      inputData:[],
      vatTempData:[]
    }
  }

  componentDidMount() {
    axios.get('http://localhost:5000/status')
    .then(res => {
      const data = res.data;
      this.setState({ inputData:data });

      const vatTemp = this.state.inputData.map(d =>
        ({x: d.timestamp, y: d.vat_temp})
      );
      this.setState({ vatTempData:vatTemp });
    })
  }

  render() {
    return (
      <div>
      {this.state.vatTempData.length}
        <VictoryChart theme={VictoryTheme.material}>
          <VictoryLine style={{
            data: { stroke: "c43a31"},
            parent: { border: "1px solid #ccc"}
          }}
          data = {this.state.vatTempData}
          />
        </VictoryChart>
      </div>
    )
  }
}

export default Chart;
