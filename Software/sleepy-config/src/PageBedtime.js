import React from 'react';

import { Button } from '@material-ui/core';
import { TimePicker } from '@material-ui/pickers';
import FormControlLabel from '@material-ui/core/FormControlLabel';
import CheckBoxIcon from '@material-ui/icons/CheckBox';
import Favorite from '@material-ui/icons/Favorite';
import FavoriteBorder from '@material-ui/icons/FavoriteBorder';

export default function PageBedtime() {
  const [alarm, setAlarm] = React.useState();

  const sendDate = () => {

  }

  return (

    <div className="screen">
      <h1>Bedtime</h1>

      <TimePicker
        autoOk
        variant="static"
        openTo="hours"
        value={alarm}
        onChange={setAlarm}
      />
      <FormControlLabel
        control={<Checkbox icon={<FavoriteBorder />} checkedIcon={<Favorite />} name="checkedH" />}
        label="Custom icon"
      />
      <Button
        variant="outlined"
        color="secondary"
        disabled={!alarm}
        onClick={sendDate}>Set alarm</Button>
    </div>
  );
}
