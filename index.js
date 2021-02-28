const receiver = require('./receiver.js');
const transmitter = require('./transmitter.js');
const connectBoard = require('./connectBoard.js');

main();

async function main(){
    await connectBoard.init(async (data) => {
        transmitter.transmitData(data);
    });
    receiver.main((data) => {
        connectBoard.receiverCb(data);
    });
}
