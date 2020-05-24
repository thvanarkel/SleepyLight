import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Switch } from '@material-ui/core';

import * as mqtt from 'react-paho-mqtt';

function App() {
  const [ client, setClient ] = React.useState(null);
  const _topic = ["Hello"];
  const _options = {};

  React.useEffect(() => {
    _init();
  },[])

  const [state, setState] = React.useState({
    turnedOn: false,
    checkedB: true,
  });

  const _init = () => {
    const c = mqtt.connect("broker.shiftr.io", Number(80), "configurator", _onConnectionLost, _onMessageArrived); // mqtt.connect(host, port, clientId, _onConnectionLost, _onMessageArrived)
    c.connect({
      onSuccess: onConnect,
      onFailure: onFailure,
      userName: '4930afd9',
      password: 'a7f2cc0b2ba3de3f'
    })
    setClient(c);

  }

  // called when sending payload
  const _sendPayload = () => {

    const payload = mqtt.parsePayload("Hello", "World"); // topic, payload
    client.send(payload);
  }

  const onConnect = () => {
    console.log("connected");
  }

  const onFailure = (e) => {
    console.log(e);
  }

  // called when client lost connection
  const _onConnectionLost = responseObject => {
    if (responseObject.errorCode !== 0) {
      console.log("onConnectionLost: " + responseObject.errorMessage);
    }
  }

  // called when messages arrived
  const _onMessageArrived = message => {
    console.log("onMessageArrived: " + message.payloadString);
  }

  // called when subscribing topic(s)
  const _onSubscribe = () => {
    client.connect({ userName: '4930afd9',
      password: 'a7f2cc0b2ba3de3f',
      onSuccess: () => {
      for (var i = 0; i < _topic.length; i++) {
        client.subscribe(_topic[i], _options);
      }}
    }); // called when the client connects
  }

  // called when subscribing topic(s)
  const _onUnsubscribe = () => {
    for (var i = 0; i < _topic.length; i++) {
      client.unsubscribe(_topic[i], _options);
    }
  }

  // called when disconnecting the client
  const _onDisconnect = () => {
    client.disconnect();
  }

  const handleChange = (event) => {
    setState({ ...state, [event.target.name]: event.target.checked });
    client.publish(event.target.name, String(event.target.checked));
  };

  return (
    <div className="App">
      <header className="App-header">
        <Switch
          checked={state.checkedA}
          onChange={handleChange}
          color="primary"
          name="turnedOn"
          inputProps={{ 'aria-label': 'secondary checkbox' }}
        />
      </header>
    </div>
  );
}

export default App;
