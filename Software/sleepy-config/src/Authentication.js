import React from 'react';
import Slide from '@material-ui/core/Slide';

import { Button, Dialog, IconButton, TextField } from '@material-ui/core'

import CloseIcon from '@material-ui/icons/Close';

import client from './mqttClient.js'

const Transition = React.forwardRef(function Transition(props, ref) {
  return <Slide direction="up" ref={ref} {...props} />;
});

export default function Authentication(props) {
  const [open, setOpen] = React.useState(true);
  const [validated, setValidated] = React.useState(false);
  const [userName, setUserName] = React.useState();
  const [password, setPassword] = React.useState();
  const [error, setError] = React.useState(false);

  React.useEffect(() => {
    if (password && userName) {
      setValidated(true);
    } else {
      setValidated(false);
    }
  })

  const connect = () => {
    client.connect({
      userName: userName,
      password: password,
      onSuccess: connected,
      onFailure: notConnected,
      useSSL:true
    })
  }

  const connected = () => {
    console.log("connected");
    props.onAuth(true);
  }

  const notConnected = (e) => {
    console.log(e);
    setError(true);
  }


  return(
    <div className="loginPage">
    <div className="login">
      <h1>Welkom</h1>
      <p>Please provide the login credentials to connect to the lamp.</p>
      <form noValidate autoComplete="off">
        <TextField
          error={error}
          id="outlined-basic"
          label="Username"
          variant="outlined"
          onChange={(e) => { setUserName(e.target.value); }} />
        <TextField
          error={error}
          id="outlined-basic"
          label="Password"
          variant="outlined"
          helperText={error == true ? 'Incorrect login credentials' : null}
          onChange={(e) => { setPassword(e.target.value); }} />
        <Button
          variant="outlined"
          color="primary"
          disabled={!validated}
          onClick={connect}>
          Connect
        </Button>
      </form>
    </div>
    </div>
    // <Dialog fullScreen open={open} onClose={handleClose} TransitionComponent={Transition}>
    //   Test
    //   <IconButton edge="start" color="inherit" onClick={handleClose} aria-label="close">
    //       <CloseIcon />
    //   </IconButton>
    // </Dialog>
  );
}
