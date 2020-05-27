import React from 'react';

import { FormControlLabel, Slider, Switch } from '@material-ui/core'

import client from './mqttClient.js'

export default function Controls() {
  const [on, setOn] = React.useState(false);
  const [value, setValue] = React.useState(0);

  const turnOn = (e, v) => {
    setOn(v);
    client.publish("/turnedOn", String(!on));
  }

  // const callback = (topic, payload) => {
  //   if (parseInt(payload) <= 0) {
  //     setOn(false)
  //   }
  //   setValue(payload)
  // }
  //
  // client.subscribe('/ledLevel', callback);

  return (
    <div className="controls">
    <FormControlLabel
      control={<Switch checked={on} onChange={turnOn} name="turnedOn" />}
      label="Light on?"
    />
    <Slider value={value}
            onChange={(e, v) => {
              setValue(v);
            }}
            onChangeCommitted={(e, v) => { client.publish("/ledLevel", String(value)); }}
            aria-labelledby="continuous-slider"
            step={5}
            min={0}
            max={1023} />
    </div>
  )
}
