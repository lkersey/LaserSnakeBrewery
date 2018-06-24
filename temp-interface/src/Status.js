import React, { Component } from 'react';
import './Status.css';
import axios from 'axios';
import moment from 'moment';

import Timestamp from './Timestamp';
class Status extends Component {

  constructor(props) {
    super(props);
    this.state = {
      fridgeTemp: 0,
      vatTemp: 0,
      timestamp: '',
      setTemp: 0,
      phase: 0
    }
  }

  getData() {
    axios.get('http://localhost:5000/status')
    .then(res => {
      const data = res.data;
      const vatTemp = data.map(d => d.vat_temp);
      const setTemp = data.map(d => d.set_temp);
      const fridgeTemp = data.map(d => d.fridge_temp);
      const timestamp = data.map(d =>
        moment(d.timestamp * 1000).format('H:mm a'));
      var phase = data.map(d=> d.phase);
      if (phase == -1.0) {
        phase = 'Error';
      } else if (phase == 1.0) {
        phase = 'Idle';
      } else if (phase == 2.0) {
        phase = 'Heating';
      } else if (phase == 3.0) {
        phase = 'Cooling';
      } else {
        phase = 'Relax';
      }
      this.setState({ fridgeTemp:fridgeTemp, vatTemp:vatTemp, phase:phase,
        timestamp:timestamp, setTemp:setTemp });
    })
  }

  componentDidMount() {
    this.getData()
    this.interval = setInterval(() => this.getData(), 1000 * 60 );
  }

  render() {
    return (
      <div className='Status'>
        <div className='wrapper'>
          <div className="box box1"><Timestamp timestamp={this.state.timestamp}/></div>
          <div className="box box2">Set temp: { this.state.setTemp }</div>
          <div className="box box3">Fridge temp: { this.state.fridgeTemp }</div>
          <div className="box box4">Vat temp: { this.state.vatTemp }</div>
          <div className="box box5">State: { this.state.phase }</div>
        </div>
      </div>
    )
  }
}

export default Status;
