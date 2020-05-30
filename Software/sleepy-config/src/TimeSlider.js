import React from 'react'

import { Slider,
  Typography } from '@material-ui/core'

export default function TimeSlider(props) {
  const marks = [
    {
      value: 0,
      label: '',
    },
    {
      value: 1,
      label: '1m',
    },
    {
      value: 5,
      label: '5m',
    },
    {
      value: 10,
      label: '10m',
    },
    {
      value: 20,
      label: '20m',
    },
    {
      value: 30,
      label: '30m',
    },
    {
      value: 45,
      label: '45m',
    },
    {
      value: 60,
      label: '1u',
    },
  ];

  return(
    <div>
    <Typography id="range-slider" variant="body1">
      {props.title}
    </Typography>
    <Typography id="range-slider" variant="subtitle2" color="textSecondary" gutterBottom>
      {props.description}
    </Typography>

    <Slider value={props.value}
            onChange={(e, v) => {
              // setTime(v);
              props.onChange(v)
            }}
            onChangeCommitted={(e, v) => { props.onChangeCommitted(v) }}
            aria-labelledby="continuous-slider"
            step={null}
            min={props.min ? props.min : 1 }
            max={props.max ? props.max: 60}
            marks={props.marks ? props.marks : marks}
            valueLabelDisplay={props.display ? props.display : "auto"}
            />
    </div>
  )
}
