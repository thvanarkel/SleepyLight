import React from 'react'
import Main from './Main';
import Authentication from './Authentication'

import client from './mqttClient.js'

function App() {
  let [authenticated, setAuthenticated] = React.useState(false);
  const [error, setError] = React.useState(false);

  const connect = (u, p) => {
    client.connect({
      userName: u,
      password: p,
      onSuccess: connected,
      onFailure: notConnected,
      useSSL:true
    })
  }

  React.useEffect(() => {
    if (!client.isConnected()) {
      setAuthenticated(false);
      // window.location.reload(true);
    }
  });

  const connected = () => {
    console.log("connected");
    setAuthenticated(true);
  }

  const notConnected = (e) => {
    console.log(e);
    setError(true);
    setAuthenticated(false);
  }

  const onSend = (u, p) => {
    connect(u, p);
  }

  return authenticated ? <Main /> : <Authentication onSend={onSend} error={error} />
  // return <Main />
}

export default App
