import React from 'react'
import Main from './Main';
import Authentication from './Authentication'

function App() {
  let [authenticated, setAuthenticated] = React.useState(false);

  const onAuth = (a) => {
    setAuthenticated(a);
  }

  return authenticated ? <Main /> : <Authentication onAuth={onAuth} />
  // return <Main />
}

export default App
