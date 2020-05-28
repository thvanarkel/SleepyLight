import React from 'react';

import client from './mqttClient.js'

export default function PageHome() {

  React.useEffect(() => {
    if (!client.isConnected()) {
      window.location.reload(true);
    }
  })

  return (
    <h1>Home</h1>
  );
}
