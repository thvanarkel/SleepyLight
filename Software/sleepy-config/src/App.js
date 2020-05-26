import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Button,
         FormControlLabel,
         Switch,
         Slider } from '@material-ui/core';

import { BottomNavigation,
         BottomNavigationAction } from '@material-ui/core'

import NightsStayIcon from '@material-ui/icons/NightsStay';
import WbSunnyIcon from '@material-ui/icons/WbSunny';
import TuneIcon from '@material-ui/icons/Tune';

import { TimePicker } from '@material-ui/pickers';

import { withStyles } from "@material-ui/core/styles";



import { moment } from 'moment';

import client from './mqttClient.js'

class App extends React.Component {
  constructor() {
    super();
    this.state = {
      alarm: null,
      turnedOn: false,
      tab: 0,
      slider: 50
    }
    client.init(); // mqtt.connect(host, port, clientId, _onConnectionLost, _onMessageArrived)
    client.connect({
      userName: '4930afd9',
      password: 'a7f2cc0b2ba3de3f'
    })
  }

  // called when sending payload


  handleChange = (event) => {
    this.setState({ turnedOn: event.target.checked });
    client.publish("/turnedOn", String(event.target.checked));
  }

  handleDateChange = (event) => {
    this.setState({alarm: event})
    // client.publish("/date", String(event.format("h:mm:ss")))
  }

  sendDate = (event) => {
    client.publish("/alarm", this.state.alarm.format("HH:mm:ss"))
  }

  setTab = (value) => {
    this.setState({tab: value});
  }

  render() {
    return (
      <div className="App">
        <header className="App-header">
          <FormControlLabel
            control={<Switch checked={this.state.turnedOn} onChange={this.handleChange} name="turnedOn" />}
            label="Light on?"
          />
          <TimePicker
            autoOk
            variant="static"
            openTo="hours"
            value={this.state.alarm}
            onChange={this.handleDateChange}
          />
          <Button
            variant="outlined"
            color="secondary"
            disabled={!this.state.alarm}
            onClick={this.sendDate}>Set alarm</Button>

          <Slider value={this.state.slider}
                  onChange={(e, v) => { this.setState({slider: v})}}
                  onChangeCommitted={(e, v) => { this.client.publish("/slider", String(this.state.slider)); }}
                  aria-labelledby="continuous-slider" />

        </header>



        <BottomNavigation
          value={this.state.tab}
          onChange={(event, newValue) => {
            console.log(newValue);
            this.setTab(newValue);
          }}
          showLabels
          // className={classes.root}
        >
        <BottomNavigationAction label="Bedtime" icon={<NightsStayIcon />} />
        <BottomNavigationAction label="Wake-up" icon={<WbSunnyIcon />} />
        <BottomNavigationAction label="Controls" icon={<TuneIcon />} />
        </BottomNavigation>

      </div>
    );
  }
}

export default App;//withStyles(styles)(App);
