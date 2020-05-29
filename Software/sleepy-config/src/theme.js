import { createMuiTheme }  from '@material-ui/core/styles'
const theme = createMuiTheme({
  palette: {
    type: 'dark',
    primary: { "main": '#f8edc8' },
    secondary: {
      "main": '#f8edc8'
    },
    background: {
      paper: '#26346d'
    }
  },
  overrides: {
    MuiBottomNavigation: {
      root: {
        // backgroundColor: "#26346d"
      }
    },
    MuiFormControlLabel: {
      root: {
        marginRight: 0
      }
    }
  }
})
export default theme
