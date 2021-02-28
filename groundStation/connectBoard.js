const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('COM3', { baudRate: 9600 });
const parser = port.pipe(new Readline({ delimiter: '\n' }));

const airId = 'airThreshold';
const tempId = 'tempThreshold';
const humidityId = 'humidityThreshold';

let air = 0;
let temp = 0;
let humidity = 0;

exports.init = async (cb) => {
    port.on('open', () => {
        console.log('Serial port open');
    });

    parser.on('data', data => {
        data = data.replace(/(\r\n|\n|\r)/gm, '');
        data = data.split(',');
        rover = data[0].split(':');
        sensor = data[1].split(':');
        // console.log("data is: ");
        // console.log(data);
        if(sensor[0] == airId){
            air = parseInt(sensor[1]);
            console.log('airThreshold', air);
            cb({
                rover: rover[1],
                airThreshold: airId + ':' + air
            });
        }
        else if(sensor[0] == tempId){
            temp = parseInt(sensor[1]);
            console.log('tempThreshold', temp);
            cb({
                rover: rover[1],
                tempThreshold: tempId + ':' + temp
            });
        }
        else if(sensor[0] == humidityId){
            humidity = parseInt(sensor[1]);
            console.log('humidityThreshold', humidity);
            cb({
                rover: rover[1],
                humidityThreshold: humidityId + ':' + humidity
            });
        }
    });

    // setInterval(async () => {
    //     cb({
    //         air: airId + ':' + air,
    //         temp: tempId + ':' + temp,
    //         humidity: humidityId + ':' + humidity
    //     });
    // }, 5000);
};

exports.receiverCb = async (data) => {/*
    if (data.air != null) {
        port.write(data.air + '\n');
        console.log("Rcvd: air:" + data.air);
    }
    if (data.temp != null) {
        port.write(data.temp + '\n');
        console.log("Rcvd: temp:" + data.temp);
    }
    if (data.humidity != null) {
        port.write(data.humidity + '\n');
        console.log("Rcvd: humidity:" + data.humidity);
    }
    if (data.score != null) {
        port.write(data.score + '\n');
        console.log("Rcvd: score:" + data.score);
    }*/
}
