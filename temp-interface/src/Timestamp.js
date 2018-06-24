import React, { Component } from 'react';
import './Timestamp.css';

class Timestamp extends Component {

  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div className="Timestamp">
        <h4>Timestamp</h4>
        { this.props.timestamp }
      </div>
    )
  }
}

export default Timestamp;
