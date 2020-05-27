import React from 'react';

import { Button } from '@material-ui/core';
import { TimePicker } from '@material-ui/pickers';

export default function PageWakeup() {
  const [alarm, setAlarm] = React.useState();

  const sendDate = () => {

  }

  return (

    <div className="screen">
      <h1>Wakeup</h1>

      <TimePicker
        autoOk
        variant="static"
        openTo="hours"
        value={alarm}
        onChange={setAlarm}
      />
      <Button
        variant="outlined"
        color="secondary"
        disabled={!alarm}
        onClick={sendDate}>Set alarm</Button>
    </div>
  );
}
