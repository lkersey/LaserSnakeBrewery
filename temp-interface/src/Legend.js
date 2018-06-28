import React, { Component } from 'react';
import './Legend.css'

class Legend extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
      <div className='Legend'>
        <div className='wrapper'>
          <hr
              style={{
                  color:'#111111',
                  backgroundColor: '#111111',
                  height: '3px',
                  width: '70px',
              }}
              align="center"
          />
          <div> Fridge Temp (&#8451;) </div>
        </div>

      </div>
    )
  }
}

export default Legend;
