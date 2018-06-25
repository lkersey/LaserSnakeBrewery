import React, { Component } from 'react';
import './StatusBox.css';

class StatusBox extends Component {

  render() {
    return (
      <div>
        <div className="title">
          { this.props.title }
        </div>
        <div className="value">
            { this.props.value }
        </div>
      </div>
    )
  }
}

export default StatusBox;
