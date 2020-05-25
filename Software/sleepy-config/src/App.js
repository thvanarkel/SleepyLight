import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Button, FormControlLabel, Switch } from '@material-ui/core';

import { TimePicker } from '@material-ui/pickers';

import * as mqtt from 'react-paho-mqtt';

import { moment } from 'moment';

class App extends React.Component {
  // const [ client, setClient ] = React.useState(null);
  // const _topic = ["Hello"];
  // const _options = {};

  constructor() {
    super();
    this.state = {
      alarm: null,
      turnedOn: false
    }
    this.client = mqtt.connect("broker.shiftr.io", Number(80), "configurator", this._onConnectionLost, this._onMessageArrived); // mqtt.connect(host, port, clientId, _onConnectionLost, _onMessageArrived)
    this.client.connect({
      onSuccess: this.onConnect,
      onFailure: this.onFailure,
      userName: '4930afd9',
      password: 'a7f2cc0b2ba3de3f'
    })
  }

  // const [state, setState] = React.useState({
  //   turnedOn: false,
  //   checkedB: true,
  // });

  // const [selectedDate, setDate] = React.useState(new Date());

  // const _init = () => {
  //
  //   setClient(c);
  //
  // }

  // called when sending payload
  _sendPayload = () => {
    const payload = mqtt.parsePayload("Hello", "World"); // topic, payload
    this.client.send(payload);
  }

  onConnect = () => {
    console.log("connected");
  }

  onFailure = (e) => {
    console.log(e);
  }

  // called when client lost connection
  _onConnectionLost = responseObject => {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost: " + responseObject.errorMessage);
    }
  }

  // called when messages arrived
  _onMessageArrived = message => {
    console.log("onMessageArrived: " + message.payloadString);
  }

  // // called when subscribing topic(s)
  // _onSubscribe = () => {
  //   this.client.connect({ userName: '4930afd9',
  //     password: 'a7f2cc0b2ba3de3f',
  //   //   onSuccess: () => {
  //   //   // for (var i = 0; i < _topic.length; i++) {
  //   //   //   this.client.subscribe(_topic[i], _options);
  //   //   // }}
  //   // }); // called when the client connects
  // }

  // // called when subscribing topic(s)
  // _onUnsubscribe = () => {
  //   for (var i = 0; i < _topic.length; i++) {
  //     this.client.unsubscribe(_topic[i], _options);
  //   }
  // }

  // called when disconnecting the client
  _onDisconnect = () => {
    this.client.disconnect();
  }

  handleChange = (event) => {
    this.setState({ turnedOn: event.target.checked });
    this.client.publish("/turnedOn", String(event.target.checked));
  }

  handleDateChange = (event) => {
    this.setState({alarm: event})
    // client.publish("/date", String(event.format("h:mm:ss")))
  }

  sendDate = (event) => {
    this.client.publish("/alarm", this.state.alarm.format("HH:mm:ss"))
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
            color="primary"
            disabled={!this.state.alarm}
            onClick={this.sendDate}>Set alarm</Button>

        </header>
      </div>
    );
  }
}

export default App;
