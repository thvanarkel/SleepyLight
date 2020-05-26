import React from 'react';

import { FormControlLabel, Slider, Switch } from '@material-ui/core'

import client from './mqttClient.js'

export default function Controls() {
  const [on, setOn] = React.useState(false);

  const turnOn = (e, v) => {
    setOn(v);
    client.publish("/turnedOn", String(on));
  }

  return (
    <div>
    <FormControlLabel
      control={<Switch checked={on} onChange={turnOn} name="turnedOn" />}
      label="Light on?"
    />
    </div>
  )
}
