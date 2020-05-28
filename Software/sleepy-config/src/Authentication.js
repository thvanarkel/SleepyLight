import React from 'react';
import Slide from '@material-ui/core/Slide';

import { Button, Dialog, IconButton, TextField } from '@material-ui/core'

import CloseIcon from '@material-ui/icons/Close';

const Transition = React.forwardRef(function Transition(props, ref) {
  return <Slide direction="up" ref={ref} {...props} />;
});

const useStateWithLocalStorage = localStorageKey => {
  const [value, setValue] = React.useState(
    localStorage.getItem(localStorageKey) || ''
  );

  React.useEffect(() => {
    localStorage.setItem(localStorageKey, value);
  }, [value]);

  return [value, setValue];
};

export default function Authentication(props) {
  const [open, setOpen] = React.useState(true);
  const [validated, setValidated] = React.useState(false);
  const [userName, setUserName] = useStateWithLocalStorage('userName');
  const [password, setPassword] = useStateWithLocalStorage('password');
  const [error, setError] = React.useState(false);

  React.useEffect(() => {
    console.log(props.error)
    setError(props.error)
    if (password && userName) {
      setValidated(true);
    } else {
      setValidated(false);
    }
  })

  const send = () => {
    setError(false);
    props.onSend(userName, password)
  }

  return(
    <div className="loginPage">
    <div className="login">
      <h1>Welkom!</h1>
      <p>Geef de inloggevens op om verbinding te maken met de lamp.</p>
      <form noValidate autoComplete="off">
        <TextField
          error={error}
          id="outlined-basic"
          label="Gebruikersnaam"
          variant="outlined"
          onChange={(e) => { setUserName(e.target.value); }}
          defaultValue={userName}
        />
        <TextField
          error={error}
          id="outlined-basic"
          label="Wachtwoord"
          variant="outlined"
          helperText={error == true ? 'Incorrect login credentials' : null}
          onChange={(e) => { setPassword(e.target.value); }}
          defaultValue={password} />
        <Button
          variant="outlined"
          color="primary"
          disabled={!validated}
          onClick={send}>
          Verbind
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
