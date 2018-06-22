import React, { Component } from 'react';
import axios from 'axios';
import moment from 'moment'
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

  getData() {
    axios.get('http://localhost:5000/status')
    .then(res => {
      const data = res.data;
      this.setState({ inputData:data });

      const vatTemp = this.state.inputData.map(d =>
        //({x: new Intl.DateTimeFormat('en-US', options).format(d.timestamp * 1000),
        ({x: moment(d.timestamp * 1000).format('dddd DD MMMM YYYY, H:mm:ss a'),
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
        <VictoryChart theme={VictoryTheme.material}>
          <VictoryLine style={{
            data: { stroke: "c43a31"},
            parent: { border: "1px solid #ccc"},
          }}
          data = {this.state.vatTempData}
          />
        </VictoryChart>
      </div>
    )
  }
}

export default Chart;
