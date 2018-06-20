import React, { Component } from 'react';
import './StatusList.css';
import Status from './Status';

import axios from 'axios';


class StatusList extends Component {

  constructor(props) {
    super(props);
    this.state = ({ vat_temps:[] });
  }

  renderStatus() {
    return this.state.vat_temps.map(vat_temp => (
      <Status vat_temp={vat_temp} />
    ));
  }

  componentDidMount() {
    axios.get('http://localhost:5000/status')
    .then(res => {
      const stats = res.data.vat_temps;
      this.setState({ vat_temps:stats });
    })
  }

  render() {
    return (
      <div className='StatusList'>
      {this.renderStatus()}
      </div>
    );
  }
}

export default StatusList;
