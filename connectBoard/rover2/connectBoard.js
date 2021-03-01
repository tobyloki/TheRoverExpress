const SerialPort = require('serialport');
const Readline = require('@serialport/parser-readline');
const port = new SerialPort('COM3', { baudRate: 9600 });
const parser = port.pipe(new Readline({ delimiter: '\n' }));

const rover1 = 'rover1';
const rover2 = 'rover2';

const airId = 'air';
const tempId = 'temp';
const humidityId = 'humidity';
const scoreId = 'score';

let air = 0;
let temp = 0;
let humidity = 0;
let score = 0;

exports.init = async (cb) => {
    port.on('open', () => {
        console.log('Serial port open');
    });

    parser.on('data', data => {
        data = data.replace(/(\r\n|\n|\r)/gm, '');
        data = data.split(':');
        // console.log(data);
        if(data[0] == airId){
            air = parseInt(data[1]);
            console.log('air', air);
            cb({
                rover: rover2,
                air: airId + ':' + air
            });
        }
        else if(data[0] == tempId){
            temp = parseInt(data[1]);
            console.log('temp', temp);
            cb({
                rover: rover2,
                temp: tempId + ':' + temp
            });
        }
        else if(data[0] == humidityId){
            humidity = parseInt(data[1]);
            console.log('humidity', humidity);
            cb({
                rover: rover2,
                humidity: humidityId + ':' + humidity
            });
        }
        else if(data[0] == scoreId){
            score = parseInt(data[1]);
            console.log('score', score);
            cb({
                rover: rover2,
                score: scoreId + ':' + score
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

exports.receiverCb = async (data) => {
    if(data.rover != null && data.rover == rover2){
        if(data.airThreshold != null){
            port.write(data.airThreshold + '\n');
            console.log("Rcvd: "+data.airThreshold);
        }
        if(data.tempThreshold != null){
            port.write(data.tempThreshold + '\n');
            console.log("Rcvd: "+data.tempThreshold);
        }
        if(data.humidityThreshold != null){
            port.write(data.humidityThreshold + '\n');
            console.log("Rcvd: "+data.humidityThreshold);
        }
    }
}
