import React from 'react';
import { makeStyles } from '@material-ui/core/styles';
import Drawer from '@material-ui/core/Drawer';
import Controls from './Controls'


import { BottomNavigation,
         BottomNavigationAction } from '@material-ui/core'

import NightsStayIcon from '@material-ui/icons/NightsStay';
import WbSunnyIcon from '@material-ui/icons/WbSunny';
import TuneIcon from '@material-ui/icons/Tune';

const useStyles = makeStyles({
  list: {
    width: 250,
  },
  fullList: {
    width: 'auto',
  },
});

export default function Navigation() {
  const classes = useStyles();
  const [drawer, setDrawer] = React.useState(false);
  const [index, setIndex] = React.useState(0);

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
      <Controls />
    </div>
  );

  return (
    <div>
      <BottomNavigation
        value={index}
        onChange={(event, newValue) => {
          if (newValue != 2) setIndex(newValue);
        }}
        showLabels
      >
      <BottomNavigationAction label="Bedtime" icon={<NightsStayIcon />} />
      <BottomNavigationAction label="Wake-up" icon={<WbSunnyIcon />} />
      <BottomNavigationAction label="Controls" icon={<TuneIcon />} onClick={toggleDrawer('bottom', true)} />
      </BottomNavigation>
      <Drawer anchor='bottom' open={drawer} onClose={toggleDrawer('bottom', false)}>
        {list()}
      </Drawer>
    </div>
  );
}
