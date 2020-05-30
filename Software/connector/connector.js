const mqtt = require('mqtt')
const dotenv = require('dotenv').config()
const path = require('path')
const fs = require('fs')
const csv = require('fast-csv')
const {
	InfluxDB,
	Point,
	FluxTableMetaData
} = require('@influxdata/influxdb-client')
const {
	hostname,
	homedir
} = require('os')

const thingName = "lamp";

const cron = require('node-cron');

// const directory = '/media/pi/' + hostname() + '/interaction-data';
const directory = homedir + '/interaction-data/';

const dbClient = new InfluxDB({
	url: process.env.HOST,
	token: process.env.TOKEN
})

class Thing {
	static write(filestream, rows, options) {
		return new Promise((res, rej) => {
			csv.writeToStream(filestream, rows, options)
				.on('error', err => rej(err))
				.on('finish', () => res());
		});
	}

	constructor(opts) {
		this.headers = opts.headers;
		this.path = opts.path;
		this.writeOpts = {
			headers: this.headers,
			includeEndRowDelimiter: true,
			rowDelimiter: "\n"
		};
		this.name = opts.name;
		this.readings = [];
		this.points = [];
		this.created = fs.existsSync(opts.path)
	}

	create(rows) {
		return Thing.write(fs.createWriteStream(this.path), rows, {
				...this.writeOpts
			})
			.then(() => this.created = fs.existsSync(this.path));
	}

	append(rows) {
		return Thing.write(fs.createWriteStream(this.path, {
			flags: 'a'
		}), rows, {
			...this.writeOpts,
			writeHeaders: false,
		});
	}

	read() {
		return new Promise((res, rej) => {
			fs.readFile(this.path, (err, contents) => {
				if (err) {
					return rej(err);
				}
				return res(contents);
			});
		});
	}

	async push() {
		// Write readings to the csv file on disk
		if (this.readings.length > 0) { // don't write empty arrays
			await Thing.write(fs.createWriteStream(this.path, {
				flags: 'a'
			}), this.readings, {
				...this.writeOpts,
				writeHeaders: false,
			});
			this.readings = [];
		}

		// Push points to the InfluxDB
		console.log(dbClient)
		const writeApi = dbClient.getWriteApi(process.env.ORG, process.env.BUCKET, 'ms')
		writeApi.useDefaultTags({
			location: hostname()
		})
		console.log(this.points);
		writeApi.writePoints(this.points)
		writeApi
			.close()
			.then(() => {
				console.log("wrote points");
				this.points = []
			})
			.catch(e => {
				console.log(e);
			});
	}

	addReading(reading) {
		if (!this.created) {
			this.create([{
				timestamp: reading.timestamp,
				stream: reading.stream,
				value: reading.value
			}])
		} else {
			this.readings.push(reading);
		}
	}

	addPoint(point) {
		this.points.push(point);
	}

	newFile() {
		let date = dateString(new Date())
		this.path = path.resolve(directory, `${this.name}-${date}.csv`)
		this.created = fs.existsSync(this.path)
	}
}

class Reading {
	constructor(timestamp, stream, value) {
		this.timestamp = timestamp;
		this.stream = stream;
		this.value = value;
	}
}

var things = [];

const connect = function() {
	if (!fs.existsSync(directory)) {
		fs.mkdirSync(directory);
	}
	client = mqtt.connect({
		host: 'broker.shiftr.io',
		port: 1883,
		username: process.env.USERN,
		password: process.env.PASSWORD,
		clientId: "connector_" + Math.random().toString(16).substr(2, 8)
	});
	client.subscribe('#');
	client.on('message', insertReading);
}

const insertReading = function(topic, payload) {
	const els = topic.split('/');
	// thingName = els[1];
	time = String(new Date().getTime());

	var data = String(payload)

	let point;

	if (els[0] === 'bedtime') {
		point = new Point(els[1])
			.tag('thing', thingName)
			.tag('moment', 'bedtime')
			.timestamp(time)
		point.stringField('value', payload)
	} else if (els[0] === 'wakeup') {
		point = new Point(els[1])
			.tag('thing', thingName)
			.tag('moment', 'wakeup')
			.timestamp(time)
		point.stringField('value', payload)
	} else {
		point = new Point(els[0])
			.tag('thing', thingName)
			.timestamp(time)
		point.intField('value', parseInt(payload));
	}
	// console.log(point);

	point.tag('thing', thingName);
	point.timestamp(time);


	// if ("tags" in data) {
	// 	for (const [key, value] of Object.entries(data.tags)) {
	// 		point.tag(key, value);
	// 	}
	// }
	// if ("fields" in data) {
	// 	for (const [key, value] of Object.entries(data.fields)) {
	// 		if (value.slice(-1) === 'i') {
	// 			point.intField(key, value)
	// 			data.fields.value = value.slice(0, -1)
	// 		} else if (value === "true" || value === "false") {
	// 			var v = value === "true"
	// 			point.booleanField(key, v)
	// 		} else if (!Number.isNaN(parseFloat(value))) {
	// 			point.floatField(key, value)
	// 		} else if (typeof value.slice(-1) === "string") {
	// 			point.stringField(key, value)
	// 		}
	// 	}
	// }

	// // Create Reading
	const reading = new Reading(time, topic, data);

	var thing = things.find(thing => thing.name === thingName)
	if (!thing) {
		let date = dateString(new Date());
		thing = new Thing({
			path: path.resolve(directory, `${thingName}-${date}.csv`),
			// headers to write
			headers: ['stream', 'timestamp', 'value'],
			name: thingName
		});
		things.push(thing);
	}
	thing.addReading(reading);
	thing.addPoint(point);
}

var dateString = function(date) {
	function pad(n) {
		return n < 10 ? '0' + n : n
	}
	let d = pad(date.getDate());
	let m = pad((date.getMonth() + 1));
	let y = date.getFullYear();
	return `${y}${m}${d}`;
}

connect();

var update = cron.schedule('*/5 * * * * *', () => {
	new Promise(resolve => {
		for (thing of things) {
			thing.push();
		}
	});
});

var newFile = cron.schedule('0 0 * * *', () => {
	for (thing of things) {
		thing.newFile();
	}
});

// const {InfluxDB, Point, HttpError} = require('@influxdata/influxdb-client')
// const dotenv = require('dotenv').config()
// const {hostname} = require('os')
//
// console.log('*** WRITE POINTS ***')
// const dbClient = new InfluxDB({
// 	url: process.env.HOST,
// 	token: process.env.TOKEN
// })
// const writeApi = dbClient.getWriteApi(process.env.ORG, process.env.BUCKET, 'ms');
// // setup default tags for all writes through this API
// writeApi.useDefaultTags({location: hostname()})
//
// // write point with the current (client-side) timestamp
// const point1 = new Point('temperature')
//   .tag('example', 'write.ts')
//   .floatField('value', 20 + Math.round(100 * Math.random()) / 10)
// writeApi.writePoint(point1)
// console.log(` ${point1}`)
// // write point with a custom timestamp
// const point2 = new Point('temperature')
//   .tag('example', 'write.ts')
//   .floatField('value', 10 + Math.round(100 * Math.random()) / 10)
//   .timestamp(new Date()) // can be also a number, but in writeApi's precision units (s, ms, us, ns)!
// writeApi.writePoint(point2)
// console.log(` ${point2.toLineProtocol(writeApi)}`)
//
// // flush pending writes and close writeApi
// writeApi
//   .close()
//   .then(() => {
//     console.log('FINISHED ... now try ./query.ts')
//   })
//   .catch(e => {
//     console.error(e)
//     if (e instanceof HttpError && e.statusCode === 401) {
//       console.log('Run ./onboarding.js to setup a new InfluxDB database.')
//     }
//     console.log('\nFinished ERROR')
//   })
