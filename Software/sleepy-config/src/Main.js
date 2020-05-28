import React from 'react';
import logo from './logo.svg';
import './App.css';

import { Button } from '@material-ui/core';

import { TimePicker } from '@material-ui/pickers';
import { withStyles } from "@material-ui/core/styles";

import Navigation from './Navigation'

import PageHome from './PageHome.js'
import PageBedtime from './PageBedtime.js'
import PageWakeup from './PageWakeup.js'

import client from './mqttClient.js'

import { moment } from 'moment';



export default function Main () {
  const [tab, setTab] = React.useState(0);

  React.useEffect(() => {
    if (!client.isConnected()) {
      window.location.reload(true);
    }
  })

  const selectPage = () => {
    let page;
    switch(tab) {
      case 0:
        return (<PageHome />);
        break;
      case 1:
        return (<PageBedtime />);
        break;
      case 2:
        return (<PageWakeup />);
        break;
    }
    return page;
  }

  return (
    <div className="App">
      {(() => {
        switch (tab) {
          case 0:
            return <PageHome />;
          case 1:
            return <PageBedtime />;
          case 2:
            return <PageWakeup />;
          default:
            return null;
      }
    })()}
      <Navigation setTab={setTab} />
    </div>
  );
}
