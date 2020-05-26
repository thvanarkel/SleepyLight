import * as mqtt from 'react-paho-mqtt'

let client;

const init = () => {
  client = mqtt.connect("broker.shiftr.io", Number(80), "configurator", _onConnectionLost, _onMessageArrived)
}

const connect = (params) => {
  client.connect(params);
}

const publish = (topic, payload) => {
  const msg = mqtt.parsePayload(topic, payload);
  client.send(msg);
}

const _sendPayload = () => {
  const payload = mqtt.parsePayload("Hello", "World"); // topic, payload
  client.send(payload);
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
const _onMessageArrived = message => {
  console.log("onMessageArrived: " + message.payloadString);
}

// // called when subscribing topic(s)
// _onSubscribe = () => {
//   this.client.connect({ userName: '4930afd9',
//     password: 'a7f2cc0b2ba3de3f',
//   //   onSuccess: () => {
//   //   // for (var i = 0; i < _topic.length; i++) {
//   //   //   this.client.subscribe(_topic[i], _options);
//   //   // }}
//   // }); // called when the client connects
// }

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
  _sendPayload,
  onConnect,
  onFailure,
  _onConnectionLost,
  _onMessageArrived,
};
export default mqttClient;
