import React from 'react';

import { Button } from '@material-ui/core';
import { TimePicker } from '@material-ui/pickers';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import FormGroup from '@material-ui/core/FormGroup';
import Checkbox from '@material-ui/core/Checkbox';
import CheckBoxIcon from '@material-ui/icons/CheckBox';
import Slider from '@material-ui/core/Slider';
import Favorite from '@material-ui/icons/Favorite';
import FavoriteBorder from '@material-ui/icons/FavoriteBorder';
import Typography from '@material-ui/core/Typography';
import Box from '@material-ui/core/Box';
import TimeSlider from './TimeSlider'

import moment from 'moment';

import client from './mqttClient.js'

import { useStateWithLocalStorage, useLocallyPersistedReducer } from './utils/persistenceHelpers'

Array.prototype.move = function(from, to) {
  this.splice(to, 0, this.splice(from, 1)[0]);
};

export default function PageBedtime() {
  const [time, setTime] =  useStateWithLocalStorage('bedtimeAlarm');
  const [reminder, setReminder] = useStateWithLocalStorage('reminder');
  const [unwind, setUnwind] = useStateWithLocalStorage('unwind', 0);
  // const [days, setDays] = React.useState();

  const [days, setDays] = useLocallyPersistedReducer(((state, newState) => ({ ...state, ...newState })),({
      'ma': false,
      'di': false,
      'wo': false,
      'do': false,
      'vr': false,
      'za': false,
      'zo': false
    }), "bedtimeDays" );

  const weekDays = ['ma', 'di', 'wo', 'do', 'vr', 'za', 'zo'];



  const setAlarm = () => {
    let msg = ""
    let week = weekDays.filter(item => item !== "zo");
    week.unshift("zo")
    week.map((d) => {
      msg += days[d] ? "1" : "0"
    });
    client.publish("/bedtime/days", msg);
    client.publish("/bedtime/alarm", moment(time).format("HH:mm:ss"))
  }

  const handleCheck = (e, d) => {
    setDays({
      [d]: e.target.checked
    })
  }

  const handleTime = (d) => {
    setTime(d.toISOString());
  }

  const sendUnwind = (v) => {
    client.publish("/unwindTime", String(v));
  }

  const sendReminder = (v) => {
    client.publish("/bedtime/reminder", String(v));
  }

  return (

    <div className="screen">
      <TimePicker
        autoOk
        variant="static"
        openTo="hours"
        value={moment(time)}
        onChange={handleTime}
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
        onClick={setAlarm}>Stel bedtijd in</Button>
      <Box mt={3}>
      <TimeSlider
        value={parseInt(reminder)}
        onChange={setReminder}
        onChangeCommitted={sendReminder}
        title="Bedtijdherinnering"
        description="Tijd voor je bedtijd dat de lamp een herinnering geeft"/>
      </Box>

      <Box mt={2}>
      <TimeSlider
        value={parseInt(unwind)}
        onChange={setUnwind}
        onChangeCommitted={sendUnwind}
        title="Ontspanningstijd"
        description="Tijd voordat de lamp uitgaat"/>
      </Box>
    </div>

  );
}
