import * as mqtt from 'react-paho-mqtt'
import os from 'os';

let client;
let callback;

const init = () => {
  client = mqtt.connect("broker.shiftr.io", Number(443), "configurator", _onConnectionLost, onMessageArrived)
}

const connect = (params) => {
  client.connect(params);
  console.log(client);
}

const publish = (topic, payload) => {
  const msg = mqtt.parsePayload(topic, payload);
  client.send(msg);
}

const isConnected = () => {
  return client.isConnected();
}

const onConnect = () => {
  console.log("connected");
}

const onFailure = (e) => {
  console.log(e);
}

// called when client lost connection
const _onConnectionLost = responseObject => {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost: " + responseObject.errorMessage);
  }
}

// called when messages arrived
const onMessageArrived = message => {
  callback(message.destinationName, message.payloadString)
}

// // called when subscribing topic(s)
const subscribe = (topic, cb) => {
  client.subscribe(topic);
  callback = cb;
}

// // called when subscribing topic(s)
// _onUnsubscribe = () => {
//   for (var i = 0; i < _topic.length; i++) {
//     this.client.unsubscribe(_topic[i], _options);
//   }
// }

// called when disconnecting the client
const _onDisconnect = () => {
  client.disconnect();
}

const mqttClient = {
  init,
  connect,
  publish,
  subscribe,
  isConnected,
  onConnect,
  onFailure,
  _onConnectionLost,
  onMessageArrived,
};
export default mqttClient;
