import React from 'react';

import { FormControlLabel, Slider, Switch } from '@material-ui/core'

import client from './mqttClient.js'
import TimeSlider from './TimeSlider'
import { useStateWithLocalStorage } from './utils/persistenceHelpers.js'

export default function Controls() {
  const [on, setOn] = React.useState(false);
  const [value, setValue] = React.useState(0);
  const [slumber, setSlumber] = useStateWithLocalStorage('slumber', 0);

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

  const sendSlumber = () => {
    client.publish("/slumberTime", String(slumber));
  }

  const marks = [
    {
      value: 0,
      label: '',
    },
    {
      value: 30,
      label: '30s',
    },
    {
      value: 60,
      label: '1m',
    },
    {
      value: 120,
      label: '2m',
    },
    {
      value: 180,
      label: '3m',
    },
    {
      value: 300,
      label: '5m',
    },
    {
      value: 600,
      label: '10m',
    },
    {
      value: 450,
      label: '15m',
    }
  ];

  return (
    <div className="controls">
    <TimeSlider
        value={parseInt(slumber)}
        onChange={setSlumber}
        onChangeCommitted={sendSlumber}
        title="Nachtstand"
        description="Tijd voordat de lamp in nachtstand uitgaat"
        marks={marks}
        min={30}
        max={450}
        display="off"/>
    </div>
  )
}
