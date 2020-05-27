import React from 'react';

import { Button } from '@material-ui/core';
import { TimePicker } from '@material-ui/pickers';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import FormGroup from '@material-ui/core/FormGroup';
import Checkbox from '@material-ui/core/Checkbox';
import CheckBoxIcon from '@material-ui/icons/CheckBox';
import Divider from '@material-ui/core/Divider';
import Favorite from '@material-ui/icons/Favorite';
import FavoriteBorder from '@material-ui/icons/FavoriteBorder';

import moment from 'moment';

import client from './mqttClient.js'

import { useStateWithLocalStorage, useLocallyPersistedReducer } from './utils/persistenceHelpers'

Array.prototype.move = function(from, to) {
  this.splice(to, 0, this.splice(from, 1)[0]);
};

export default function PageBedtime() {
  const [time, setTime] =  useStateWithLocalStorage('bedtimeAlarm');
  // const [days, setDays] = React.useState();

  const [days, setDays] = useLocallyPersistedReducer(((state, newState) => ({ ...state, ...newState })),({
      'mon': false,
      'tue': false,
      'wed': false,
      'thu': false,
      'fri': false,
      'sat': false,
      'sun': false
    }), "bedtimeDays" );

  const weekDays = ['mon', 'tue', 'wed', 'thu', 'fri', 'sat', 'sun'];

  const setAlarm = () => {
    let msg = ""
    let week = weekDays.filter(item => item !== "sun");
    week.unshift("sun")
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

  return (

    <div className="screen">
      <h1>Bedtime</h1>

      <TimePicker
        autoOk
        variant="static"
        openTo="hours"
        value={moment(time)}
        onChange={handleTime}
      />
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

      <Button
        variant="outlined"
        color="secondary"
        disabled={!time}
        onClick={setAlarm}>Set alarm</Button>

      <Divider light />
      <p>Test</p>
    </div>

  );
}
