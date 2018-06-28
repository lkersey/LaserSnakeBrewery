import React, { Component } from 'react';
import laserSnakeLogo from './laser-snake-logo.svg';
import './LogoDisplay.css'

class LogoDisplay extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div className="LogoDisplay">
        <img src={laserSnakeLogo} alt=""
        className="LogoDisplay"/>
      </div>
    )
  }
}

export default LogoDisplay;
