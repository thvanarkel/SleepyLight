import React from 'react';

import client from './mqttClient.js'

import { IconButton, Drawer } from '@material-ui/core'
import HelpOutline from '@material-ui/icons/HelpOutline';
import AwesomeSlider from 'react-awesome-slider';
import 'react-awesome-slider/dist/styles.css';
import Box from '@material-ui/core/Box';

import help01 from './images/help01.png'
import help02 from './images/help02.png'
import help03 from './images/help03.png'


export default function PageHome() {
  const [drawer, setDrawer] = React.useState(false);

  React.useEffect(() => {
    if (!client.isConnected()) {
      window.location.reload(true);
    }
  })

  const toggleDrawer = (anchor, open) => (event) => {
    if (event.type === 'keydown' && (event.key === 'Tab' || event.key === 'Shift')) {
      return;
    }
    setDrawer( open );
  };

  const list = () => (
    <div

      role="presentation"
      // onClick={toggleDrawer(false)}
      onKeyDown={toggleDrawer(false)}
    >
    <Box mb={4}>
    <AwesomeSlider bullets={true} organicArrows={true}>
      <div><img src={help01}/></div>
      <div><img src={help02}/></div>
      <div><img src={help03}/></div>
    </AwesomeSlider>
    </Box>
    </div>
  );

  return (
    <div className="screen">
      <div className="header">
      <h1>Home</h1>
      <IconButton color="primary" aria-label="add to shopping cart" onClick={toggleDrawer('top', true)}>
        <HelpOutline />
      </IconButton>
      <Drawer anchor='top' open={drawer} onClose={toggleDrawer('top', false)}>
        {list()}
      </Drawer>
      </div>
    </div>
  );
}
