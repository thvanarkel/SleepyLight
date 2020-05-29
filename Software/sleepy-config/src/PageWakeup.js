import React from 'react';

import { Button } from '@material-ui/core';
import { TimePicker } from '@material-ui/pickers';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import FormGroup from '@material-ui/core/FormGroup';
import Checkbox from '@material-ui/core/Checkbox';
import CheckBoxIcon from '@material-ui/icons/CheckBox';
import Favorite from '@material-ui/icons/Favorite';
import FavoriteBorder from '@material-ui/icons/FavoriteBorder';
import Box from '@material-ui/core/Box';
import TimeSlider from './TimeSlider'
import { FormControl,
         Select,
         MenuItem,
         FormHelperText } from '@material-ui/core'

import moment from 'moment';

import client from './mqttClient.js'

import { useStateWithLocalStorage, useLocallyPersistedReducer } from './utils/persistenceHelpers'

Array.prototype.move = function(from, to) {
  this.splice(to, 0, this.splice(from, 1)[0]);
};

export default function PageHome() {
  const [time, setTime] = useStateWithLocalStorage('wakeAlarm');
  const [awake,setAwake] = useStateWithLocalStorage('awakeTime', 30);
  const [snooze,setSnooze] = useStateWithLocalStorage('snooze', 8);
  const [sound, setSound] = useStateWithLocalStorage('sound', 2);
  // const [days, setDays] = React.useState();

  const [days, setDays] = useLocallyPersistedReducer(((state, newState) => ({ ...state, ...newState })),({
      'ma': false,
      'di': false,
      'wo': false,
      'do': false,
      'vr': false,
      'za': false,
      'zo': false
    }), "wakeupDays" );

  React.useEffect(() => {
    if (!client.isConnected()) {
      window.location.reload(true);
    }
  })

  const weekDays = ['ma', 'di', 'wo', 'do', 'vr', 'za', 'zo'];

  const setAlarm = () => {
    let msg = ""

    let week = weekDays.filter(item => item !== "zo");
    week.unshift("zo")

    week.map((d) => {
      msg += days[d] ? "1" : "0"
    });
    client.publish("/wakeup/days", msg);
    client.publish("/wakeup/alarm", moment(time).format("HH:mm:ss"))
  }

  const handleCheck = (e, d) => {
    setDays({
      [d]: e.target.checked
    })
  }

  const handleTime = (d) => {
    setTime(d.toISOString());
  }

  const sendAwakeTime = (v) => {
    client.publish("/awakeTime", String(v));
  }

  const sendSnooze = (v) => {
    client.publish("/snooze", String(v));
  }

  const handleSound = (e) => {
    setSound(e.target.value);
    client.publish("/sound", String(e.target.value));
  }

  const marks = [
    {
      value: 2,
      label: '2m',
    },
    {
      value: 3,
      label: '3m',
    },
    {
      value: 5,
      label: '5m',
    },
    {
      value: 8,
      label: '8m',
    },
    {
      value: 10,
      label: '10m',
    },
    {
      value: 15,
      label: '15m',
    }
  ];

  return (

    <div className="screen">
      <TimePicker
        autoOk
        variant="static"
        openTo="hours"
        ampm={false}
        value={moment(time)}
        onChange={setTime}
      />
      <Box mx="auto" mt={2}>
      <FormGroup aria-label="position" row>
      {weekDays.map((d) => (
        <React.Fragment key={d}>
          <FormControlLabel
              value="top"
              control={<Checkbox color="primary" checked={days[d]} onChange={e => handleCheck(e,d)}  />}
              label={d}
              labelPlacement="top"
            />
        </React.Fragment>
      ))}
      </FormGroup>
      </Box>

      <Button
        variant="outlined"
        color="secondary"
        disabled={!time}
        onClick={setAlarm}>Stel opstaan alarm in</Button>

      <Box mt={3}>
      <TimeSlider
        value={parseInt(awake)}
        onChange={setAwake}
        onChangeCommitted={sendAwakeTime}
        title="Ontwakingstijd"
        description="Tijd voor je alarm dat de lamp aan gaat"/>
      </Box>
      <Box mt={2}>
      <TimeSlider
          value={parseInt(snooze)}
          onChange={setSnooze}
          onChangeCommitted={sendSnooze}
          title="Snooze tijd"
          description="Tijd die je per keer je wekker kunt uitstellen"
          marks={marks}
          min={2}
          max={15}/>
      </Box>
      <Box mt={2}>
      <FormControl>
        <Select
          value={sound}
          onClose={handleSound}
          displayEmpty
        >
          <MenuItem value={0}>Birdsong</MenuItem>
          <MenuItem value={1}>Bright</MenuItem>
          <MenuItem value={2}>Droplets</MenuItem>
          <MenuItem value={3}>Early</MenuItem>
          <MenuItem value={4}>Helios</MenuItem>
          <MenuItem value={6}>Spring</MenuItem>
          <MenuItem value={7}>Sunny</MenuItem>
        </Select>
        <FormHelperText>Geluid</FormHelperText>
      </FormControl>
      </Box>

    </div>
  );
}
