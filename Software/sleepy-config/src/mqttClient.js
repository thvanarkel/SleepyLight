import mqtt from 'mqtt';
const broker = 'mqtt://4930afd9:a7f2cc0b2ba3de3f@broker.shiftr.io';

function getClient(errorHandler) {
  const client = mqtt.connect(broker, {
    clientId: 'configurator'
  });
  client.stream.on('error', (err) => {
    errorHandler(`Connection to ${broker} failed`);
    client.end();
  });
  return client;
}

function subscribe(client, topic, errorHandler) {
  const callBack = (err, granted) => {
    if (err) {
      errorHandler('Subscription request failed');
    }
  };
  return client.subscribe(apiEndpoint + topic, callBack);
}

function onMessage(client, callBack) {
  client.on('message', (topic, message, packet) => {
    callBack(JSON.parse(new TextDecoder('utf-8').decode(message)));
  });
}

function unsubscribe(client, topic) {
  client.unsubscribe(apiEndpoint + topic);
}

function publish(client, topic, message) {

}

function closeConnection(client) {
  client.end();
}

const mqttService = {
  getClient,
  subscribe,
  onMessage,
  unsubscribe,
  closeConnection,
};
export default mqttService;
