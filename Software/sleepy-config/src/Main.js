import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Button } from '@material-ui/core';

import { BottomNavigation,
         BottomNavigationAction } from '@material-ui/core'

import NightsStayIcon from '@material-ui/icons/NightsStay';
import WbSunnyIcon from '@material-ui/icons/WbSunny';
import TuneIcon from '@material-ui/icons/Tune';

import { TimePicker } from '@material-ui/pickers';
import { withStyles } from "@material-ui/core/styles";

import Navigation from './Navigation.js'

import Authentication from './Authentication.js'

import { moment } from 'moment';



class Main extends React.Component {
  constructor() {
    super();
    this.state = {
      authenticated: false,
      alarm: null,
      turnedOn: false,
      tab: 0,
      slider: 50
    }
  }

  // called when sending payload


  handleChange = (event) => {
    this.setState({ turnedOn: event.target.checked });

  }

  handleDateChange = (event) => {
    this.setState({alarm: event})
    // client.publish("/date", String(event.format("h:mm:ss")))
  }

  sendDate = (event) => {
    // client.publish("/alarm", this.state.alarm.format("HH:mm:ss"))
  }

  setTab = (value) => {
    this.setState({tab: value});
  }


  render() {
    return (
      <div className="App">

        <div className="screen">


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
        </div>

        <Navigation />
      </div>
    );
  }
}

export default Main;//withStyles(styles)(App);
