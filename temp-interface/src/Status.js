import React, { Component } from 'react';
import './Status.css';
import axios from 'axios';
import moment from 'moment';
import StatusBox from './StatusBox';
import laserSnakeLogo from './laser-snake-logo.svg';

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
    axios.get('https://lasersnake.duckdns.org/api/status')
    .then(res => {
      const data = res.data;
      const vatTemp = data.map(d => d.vat_temp);
      const setTemp = data.map(d => d.set_temp);
      const fridgeTemp = data.map(d => d.fridge_temp);
      const timestamp = data.map(d =>
        moment(d.timestamp * 1000).format('MMM Do H:mm a'));
      var phase = data.map(d=> d.phase);
      if (phase == -1.0) {
        phase = 'Error';
      } else if (phase == 1.0) {
        phase = 'Idle';
      } else if (phase == 2.0) {
        phase = 'Cooling';
      } else if (phase == 3.0) {
        phase = 'Heating';
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
        <div className='wrapper'>

          <div className='box logo'>
            <img src={laserSnakeLogo} alt=""/>
          </div>

          <div className="box box1">
            <StatusBox title="last updated"
              value={this.state.timestamp}/>
          </div>

          <div className="box box2">
            <StatusBox title='Set temp'
              value={ this.state.setTemp + '\u2103'} />
          </div>

          <div className="box box3">
            <StatusBox title='Fridge temp'
              value={ this.state.fridgeTemp + '\u2103'} />
          </div>

          <div className="box box4">
            <StatusBox title='Vat temp'
              value={ this.state.vatTemp + '\u2103'} />
          </div>

          <div className="box box5">
            <StatusBox title='State'
              value={ this.state.phase } />
          </div>

        </div>
    )
  }
}

export default Status;
