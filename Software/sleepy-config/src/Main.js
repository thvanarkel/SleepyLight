import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Button } from '@material-ui/core';

import { TimePicker } from '@material-ui/pickers';
import { withStyles } from "@material-ui/core/styles";

import Navigation from './Navigation'

import PageHome from './PageHome.js'
import PageBedtime from './PageBedtime.js'
import PageWakeup from './PageWakeup.js'


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

  selectPage = (i) => {

  }


  render() {
    let page;
    switch(this.state.tab) {
      case 0:
        page = <PageHome />;
        break;
      case 1:
        page = <PageBedtime />;
        break;
      case 2:
        page = <PageWakeup />;
        break;
    }
    return (
      <div className="App">
        {page}


        <Navigation setTab={this.setTab} />
      </div>
    );
  }
}

export default Main;//withStyles(styles)(App);
