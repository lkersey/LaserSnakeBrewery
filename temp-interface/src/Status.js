import React, { Component } from 'react';
import './Status.css'

class Status extends Component {

  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div className='Status'>
      { this.props.vat_temp }
      </div>
    )
  }
}

export default Status;
